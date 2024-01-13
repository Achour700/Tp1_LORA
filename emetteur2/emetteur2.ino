/*
 * Auteurs: HOUIMEL Achour et Najmi Ilyass
 */

#include <SPI.h>
#include <LoRa.h>

#define MAX_DATA_SIZE 247   // Taille maximale en octets des données
#define MAX_TRAM_SIZE 255   // Taille maximale de la trame
#define DEMAND_ID_TRAM 0    // Trame de demande d'ID
#define OUT_OF_NET_TRAM 2   // Trame informant la sortie du nœud
#define SEND_DATA_TRAM 3    // Trame d'envoi de données

volatile unsigned char data[MAX_DATA_SIZE];
volatile unsigned char sendingTram[MAX_TRAM_SIZE]; // Trame à envoyer - pour faciliter la reconstitution
String receivedTram = "";                           // Trame reçue - sous forme de string pour faciliter le traitement de la Trame
volatile unsigned char localID = 0; // Adresse locale du nœud 
unsigned char destID = 1;           // Adresse du destinataire - Collecteur
String senderID = "";               // Adresse émetteur / Adresse affectée
int nodeID;

// Fonction pour envoyer une Trame
void sendTram(unsigned char *tram) {
  unsigned int i = 0;
  LoRa.beginPacket();
  while (tram[i] != 0xFF) {
    LoRa.print(tram[i]);
    i++;
  }
  LoRa.endPacket();
}

void setup() {
  // Initialisation du port série & LoRa
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Test de début :");
  if (!LoRa.begin(866.7E6)) {
    Serial.println("Démarrage de LoRa échoué !");
    while (1);
  }
  Serial.println("Demande d'identification.");
  
  // Construction de la Trame de demande d'ID
  sendingTram[0] = DEMAND_ID_TRAM; // Fonction
  sendingTram[1] = localID;        // Adresse locale = 0 (pas encore affecté)
  sendingTram[2] = destID;         // Adresse destination = 1
  sendingTram[3] = 0xFF;           // 0xFF pour indiquer la fin de la Trame / nécessaire au niveau du collecteur
  
  // Appel à la fonction sendTram pour envoyer la Trame
  sendTram(&sendingTram[0]);
  
  // Recevoir la Trame d'Identifiant
  int packetSize = LoRa.parsePacket();
  while (!LoRa.available()); // Attend dans la boucle jusqu'à l'arrivée d'une Trame
  Serial.print("Paquet reçu '");
  
  // Lecture de la Trame
  while (LoRa.available()) {
    receivedTram += (char)LoRa.read();
  }
  Serial.print(receivedTram);
  int receivedTramSize = receivedTram.length();
  
  // Affichage du RSSI du paquet
  Serial.print("' avec RSSI ");
  Serial.println(LoRa.packetRssi());
  
  // Extraction de l'adresse de l'émetteur
  senderID = String(receivedTram[2]); // Adresse émetteur / Identifiant affecté
  Serial.print("Adresse émetteur reçue est : ");
  Serial.println((char)senderID[0]);
  
  // Conversion de l'ID en entier pour éviter le problème d'affichage en réception (ex. ID = 2 => ID_reçu = 50)
  for (int i = 2; i < 10; i++) {
    if (senderID == (String)i) {
      nodeID = i;
      Serial.print("ID : ");
      Serial.println(nodeID);
      break;
    }
  }
  
  // Fixation de la première partie de la Trame (Fonction + adresse_émetteur + adresse de destination)
  sendingTram[0] = SEND_DATA_TRAM; // Fonction
  sendingTram[1] = nodeID;          // Adresse locale : identifiant affecté
  sendingTram[2] = destID;          // Adresse destination = 1
  
  // À partir de l'indice 3, on peut mettre des données + la fin de la Trame est 0xFF
  Serial.println("Temperature:,Humidity:"); // Libellés pour le tracé
}

void loop() {
  int counter = 0;
  for (int i = 0; i < 3; i++) { // Envoyer trois mesures de température puis demander la sortie
    // Générer des valeurs aléatoires de température et d'humidité
    float temperature = random(10, 40);
    float humidity = random(10, 99);

    // Mettre les valeurs dans la Trame
    sendingTram[3] = temperature;
    sendingTram[4] = humidity;
    sendingTram[5] = 0xFF;
    
    Serial.print("Temperature : ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity : ");
    Serial.print(humidity);
    Serial.println(" %");
    Serial.println();
    
    // Envoyer la Trame
    sendTram(&sendingTram[0]);

    counter++;
    delay(5000);
    
    if (counter == 3) { // Après 1000 mesures, envoyer la demande de sortie du nœud
      // Mettre la fonction de la Trame à 2 (sortie du réseau)
      sendingTram[0] = OUT_OF_NET_TRAM;

      // Envoyer la Trame
      sendTram(&sendingTram[0]);

      Serial.println("Sortie du réseau. Attendez 10 secondes");
      delay(10000);
      asm volatile("jmp 0"); // Redémarrer la carte pour faire une demande d'authentification 
      delay(5000);
      // while (1); // Attendre la réinitialisation
    }
  }
}
