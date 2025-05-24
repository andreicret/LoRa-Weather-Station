#include <LoRa.h>
#include <Wire.h>
#include <BH1750.h>
#include <SHT2x.h> 

BH1750 lightMeter;
SHT2x sht; 
const int LoRa_FREQ = 433E6;
int counter = 0;

void setup() {

  Wire.begin();
  Serial.begin(9600);
  while(!Serial);


  Serial.print("Init\n");
  Serial.println("LoRa Receiver");

  //Init LoRa module
  if (!LoRa.begin(LoRa_FREQ)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }  
}

void loop() {

  // Receive data via LoRa
  int packetSize = LoRa.parsePacket();
  if (packetSize) {

    Serial.print("Received packet! ");

    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}



