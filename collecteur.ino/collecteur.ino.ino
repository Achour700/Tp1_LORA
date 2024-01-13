#include <SPI.h>
#include <LoRa.h>

#define DEMAND_ID  0
#define OUT_OF_NET 2
#define SEND_DATA  3
#define COLLECTEUR_ID 1
#define SIZE_OF_TRAM 5

int nextID = 1;
volatile int sendingTram[SIZE_OF_TRAM];
volatile int receivedTram[SIZE_OF_TRAM];

// Structure de données pour stocker les relations entre emetteurID et assignedID
struct IdMapping {
  int emetteurID;
  int assignedID;
};

IdMapping idMappings[SIZE_OF_TRAM];

int sendTram(int *tram) {
  int i = 0;
  LoRa.beginPacket();
  while (tram[i] != 0xFF) {
    LoRa.write(tram[i]);
    i++;
  }
  LoRa.endPacket();
  return 1;
}

int assignID(int emetteurID) {
  for (int i = 0; i < SIZE_OF_TRAM; i++) {
    if (idMappings[i].emetteurID == emetteurID) {
      // L'ID a déjà été attribué, renvoyer l'ID existant
      return idMappings[i].assignedID;
    }
  }

  // L'ID n'a pas encore été attribué, attribuer un nouvel ID
  for (int i = 0; i < SIZE_OF_TRAM; i++) {
    if (idMappings[i].emetteurID == 0) {
      idMappings[i].emetteurID = emetteurID;
      idMappings[i].assignedID = nextID++;
      return idMappings[i].assignedID;
    }
  }

  // Gérer le dépassement de la taille de idMappings si nécessaire
  return 0;
}

int handleDemandID() {
  Serial.print("Trame de demande d'identification : ");
  Serial.println(receivedTram[0]);
  Serial.println("----------------------------------------- ");
  int emetteurID = receivedTram[1];
  int assignedID = assignID(emetteurID);
  Serial.print("ID attribué à l'émetteur : ");
  Serial.println(assignedID);
  Serial.println("----------------------------------------- ");
  Serial.println("----------------------------------------- ");
  Serial.println("");

  sendingTram[0] = assignedID;
  sendingTram[1] = 0xFF;
  sendingTram[2] = 0xFF;
  sendTram(sendingTram);
  return 1;
}

int handleNodeDisconnected() {
  Serial.println("############### ");
  Serial.println("Noeud déconnecté ");
  Serial.println("############### ");
  Serial.println();
  return 1;
}

int handleSendData() {
  Serial.print("Données envoyées par le noeud émetteur : ");
  for (int i = 0; i < SIZE_OF_TRAM; i++) {
    Serial.print(receivedTram[i]);
  }
  Serial.println();
  Serial.println("------------------------------------");
  Serial.println();
  int emetteurID = receivedTram[1];
  int assignedID = assignID(emetteurID);
  Serial.print("ID du l'émetteur : ");
  Serial.println(assignedID);
  Serial.print("Temperature : ");
  Serial.println(receivedTram[3]);
  Serial.print("Humidité : ");
  Serial.println(receivedTram[4]);
  Serial.println();
  Serial.println("------------------------------------");

  int temperature = receivedTram[3];
  int humidity = receivedTram[4];
  return 1;
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("############### ");
  Serial.println("Collecteur LoRa");
  Serial.println("###############");
  Serial.println(" ");
  Serial.println(" ");

  if (!LoRa.begin(863.7E6)) {
    Serial.println("Démarrage de LoRa échoué !");
    while (1);
  }
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while (LoRa.available()) {
      for (int i = 0; i < SIZE_OF_TRAM; i++) {
        receivedTram[i] = LoRa.read();
      }
      int tramType = receivedTram[0];
      int emetteurID = receivedTram[1];

      if (tramType == DEMAND_ID && (int)receivedTram[2] == 1) {
        // Vérifier si ID est attribué
        int assignedID = assignID(emetteurID);
        if (assignedID == 0) {
          Serial.println("Attendez l'attribution de l'ID.");
          return; // Arrêter le traitement de la tram
        }
        handleDemandID();
      } else if (tramType == OUT_OF_NET && (int)receivedTram[2] == 1) {
        handleNodeDisconnected();
      } else if (tramType == SEND_DATA && (int)receivedTram[2] == 1) {
        // Vérifier si ID est attribué
        int assignedID = assignID(emetteurID);
        if (assignedID == 0) {
          Serial.println("Attendez l'attribution de l'ID.");
          return; // Arrêter le traitement de la tram
        }
        handleSendData();
      } else {
        // Gérer d'autres cas ou ignorer les trams non reconnues
      }
    }
  }
}
