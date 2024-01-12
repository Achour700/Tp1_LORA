#include <SPI.h>
#include <LoRa.h>

#define DEMAND_ID  0
#define OUT_OF_NET 2
#define SEND_DATA  3
#define COLLECTEUR_ID 1
#define EMETTEUR_ID 3

#define SIZE_OF_TRAM 6

int emetteurID = EMETTEUR_ID;
int collecteurID = COLLECTEUR_ID;
int temperature = 0;
int humidity = 0;
int sendingTram[SIZE_OF_TRAM];
int receivedTram[SIZE_OF_TRAM];
int dataSent = 0;  // Variable pour suivre le nombre de données envoyées

int sendTram(int *tram) {
  Serial.print("Trame envoyée : ");
  int i = 0;
  LoRa.beginPacket();
  while (tram[i] != 0xFF) {
    LoRa.write(tram[i]);
    Serial.print(tram[i]);
    i++;
  }
  LoRa.endPacket();
  Serial.println();
  return 1;
}

int demandID() {
  sendingTram[0] = DEMAND_ID;
  sendingTram[1] = emetteurID;
  sendingTram[2] = collecteurID;
  sendingTram[3] = 0xFF;
  sendingTram[4] = 0xFF;
  sendingTram[5] = 0xFF;
  return sendTram(sendingTram);
}

int sendData() {
  sendingTram[0] = SEND_DATA;
  sendingTram[1] = emetteurID;
  sendingTram[2] = collecteurID;
  sendingTram[3] = temperature;
  sendingTram[4] = humidity;
  sendingTram[5] = 0xFF;
  return sendTram(sendingTram);
}

int outOfNet() {
  sendingTram[0] = OUT_OF_NET;
  sendingTram[1] = emetteurID;
  sendingTram[2] = collecteurID;
  sendingTram[3] = 0xFF;
  sendingTram[4] = 0xFF;
  sendingTram[5] = 0xFF;
  return sendTram(sendingTram);
}

void generateRandomData() {
  temperature = random(0, 40);
  humidity = random(0, 101);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Emetteur LoRa");

  if (!LoRa.begin(863.7E6)) {
    Serial.println("Démarrage de LoRa échoué !");
    while (1);
  }

  if (emetteurID == 0) {
    demandID();
  }
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Paquet reçu du collecteur : ");
    while (LoRa.available()) {
      for (int i = 0; i < SIZE_OF_TRAM; i++) {
        receivedTram[i] = LoRa.read();
        Serial.print(receivedTram[i]);
      }
      Serial.println();
      int tramType = receivedTram[0];
      if (tramType == DEMAND_ID) {
        emetteurID = receivedTram[1];
        Serial.print("ID reçu du collecteur : ");
        Serial.println(emetteurID);
      } else {
        Serial.println("Trame non reconnue ou non destinée à cet émetteur");
      }
    }
  }

  // Envoyer les données seulement si nous n'avons pas atteint la limite
  if (dataSent < 10) {
    generateRandomData();
    sendData();
    dataSent++;
    delay(2000);
  }

  // Une fois que nous avons envoyé 10 fois, simuler une déconnexion du noeud
  if (dataSent == 10) {
    outOfNet();
    Serial.print("Emetteur déconnecté et va se reconnecter dans 10 secondes ");
    delay(10000);

    // Réinitialiser la variable et se reconnecter au collecteur
    dataSent = 0;
    demandID();
  }
}
