/*
   Auteurs: HOUIMEL Achour et Najmi Ilyass
*/

#include <SPI.h>
#include <LoRa.h>

#define MAX_DATA_SIZE 247
#define MAX_TRAM_SIZE 255
#define Affect_ID_TRAM 1 // Trame d'affectation d'ID

unsigned char local_ID = 1; // Identifiant local du nœud collecteur
String receivedTram = "";   // Variable pour stocker la tram reçue sous forme de String
char sendingTram[MAX_TRAM_SIZE] = {0};  // Trame à envoyer
char data[MAX_DATA_SIZE];   // Données à transmettre
char node_ID[8] = {0};      // Tableau pour stocker les identifiants de nœuds affectés
String Addresses[8] = {""}; // Tableau pour enregistrer les adresses de 8 nœuds
String CurrentAddress = ""; // Variable pour stocker l'adresse actuelle lors de la réception d'une demande d'identification
String node_IDentifier = ""; // Variable pour enregistrer l'identifiant lors de la réception d'une demande de sortie du réseau
String Data = "";           // Variable pour enregistrer les données reçues lors de la réception d'une tram de données
volatile int node_count = 0; // Nombre actuel de nœuds enregistrés

/**
   @brief Obtient l'adresse à partir de la tram.
   @param tram Trame reçue.
   @return Adresse extraite de la tram.
*/
String obtain_Address(String tram) {
  String address = (String)tram[1];
  return address;
}

/**
   @brief Obtient l'identifiant à partir de la tram.
   @param tram Trame reçue.
   @return Identifiant extrait de la tram.
*/
String obtain_ID(String tram) {
  String ID = "";
  ID += (String)tram[1];
  return ID;
}

/**
   @brief Obtient les données à partir de la tram.
   @param tram Trame reçue.
   @return Données extraites de la tram.
*/
String obtain_data(String tram) {
  String data = (String)tram[3] + (String)tram[4] + (String)tram[5] + (String)tram[6];
  return data;
}

/**
   @brief Vérifie l'existence d'une adresse dans le tableau.
   @param address Adresse à vérifier.
   @return 1 si l'adresse existe, 0 sinon.
*/
int check_existence(String address) {
  for (int i = 0; i < 8; i++) {
    if (Addresses[i] == address) {
      Serial.print("Adresse trouvée : ");
      Serial.println(address);
      return 1;
    }
  }
  Serial.print("Nouveau nœud - Adresse : ");
  Serial.println(address);
  return 0;
}

/**
   @brief Obtient l'indice d'une adresse dans le tableau.
   @param address Adresse dont on veut obtenir l'indice.
   @return Indice de l'adresse dans le tableau.
*/
int get_index(String address) {
  for (int i = 0; i < 8; i++) {
    if (Addresses[i] == address)
      return i;
  }
  return 8;
}

/**
   @brief Configure la tram d'envoi avec l'ID et l'adresse locale.
   @param Tram_ID Identifiant de la tram.
   @param Node_ID Identifiant du nœud.
*/
void configure_Tram(char Tram_ID, char Node_ID) {
  sendingTram[0] = Tram_ID;
  sendingTram[1] = local_ID;
  sendingTram[2] = Node_ID;
  sendingTram[3] = 0xFF;
}

/**
   @brief Envoie une tram.
   @param Tram Tram à envoyer.
*/
void send_Tram(unsigned char *Tram) {
  unsigned int i = 0;
  LoRa.beginPacket();
  while (Tram[i] != 0xFF) {
    LoRa.print(Tram[i]);
    i++;
  }
  LoRa.endPacket();
}

/**
   @brief Initialisation du nœud collecteur.
*/
void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("Nœud Collecteur");
  Serial.println("--------------------------------");
  Serial.println("Auteurs: HOUIMEL Achour et Najmi Ilyass");
  if (!LoRa.begin(866.7E6)) {
    Serial.println("Initialisation échouée");
    while (1)
      ;
  }
}

/**
   @brief Boucle principale du nœud collecteur.
*/
void loop() {
  int packet_size = LoRa.parsePacket();
  if (packet_size) {
    Serial.println("Paquet reçu:");
    int i = 0;
    while (LoRa.available()) {
      receivedTram += (char)LoRa.read();
    }
    Serial.println(receivedTram);

    switch (receivedTram[0]) {
      case '0':
        Serial.println("Tram reçue, type : Demande d'identifiant");
        CurrentAddress = obtain_Address(receivedTram);
        if (check_existence(CurrentAddress) == 1) {
          int ind = get_index(CurrentAddress);
          if (ind != 8) {
            configure_Tram(Affect_ID_TRAM, node_ID[ind]);
          } else
            configure_Tram(Affect_ID_TRAM, 0);
          send_Tram(&sendingTram[0]);
          Serial.println("Tram d'identification envoyée.");
          receivedTram = "";
          break;
        }
        if (node_count < 8) {
          Addresses[node_count] = CurrentAddress;
          node_ID[node_count] = (char)(node_count + 2);
          node_count++;
        }
        configure_Tram(Affect_ID_TRAM, node_ID[node_count - 1]);
        send_Tram(&sendingTram[0]);
        Serial.println("Tram d'identification envoyée.");
        Serial.println("--------------------------------");
        Serial.println("--------------------------------");
        receivedTram = "";
        break;
      case '2':
        node_IDentifier = obtain_ID(receivedTram);
        Serial.print("Demande de sortie du réseau de la part de : ");
        Serial.println(node_IDentifier);
        for (int i = 0; i < 8; i++) {
          if (node_ID[i] == node_IDentifier[1]) {
            node_ID[i] = 0;
            Addresses[i] = "";
            node_count = i;
            Serial.println("Nœud supprimé.");
            Serial.println("--------------------------------");
            Serial.println("--------------------------------");
            break;
          }
        }
        receivedTram = "";
        break;
      case '3':
        Serial.println("Tram reçue, type : Tram de données");
        node_IDentifier = obtain_ID(receivedTram);
        Serial.print("ID du nœud : ");
        Serial.println(node_IDentifier);
        Data = obtain_data(receivedTram);

        Serial.print("Température : ");
        Serial.print((String)receivedTram[3

                                         ]);
        Serial.print((String)receivedTram[4]);
        Serial.println(" °C");

        Serial.print("Humidité : ");
        Serial.print((String)receivedTram[5]);
        Serial.print((String)receivedTram[6]);
        Serial.println(" %");

        Serial.println("--------------------------------");

        receivedTram = "";
        break;
      default:
        break;
    }
  }
}
