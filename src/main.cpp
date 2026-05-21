#include <Arduino.h>
#include "../lib/includes.h"

MFRC522 rfid(SS_PIN, RST_PIN); // Create an instance of the MFRC522 class
unsigned long lastRead = 0;
const unsigned long RESET_INTERVAL = 10000;

void setup() {
  Serial.begin(115200); // Initialize serial communication
  Serial.println("Sistema inicianod - Gestion de turnos");
  SPI.begin(); // Initialize SPI bus
  rfid.PCD_Init(); // Initialize the MFRC522 RFID reader
}

void loop() {
  // put your main code here, to run repeatedly:

  if (millis() - lastRead > RESET_INTERVAL) {
    rfid.PCD_Init();
    lastRead = millis();
  }

  if (!rfid.PICC_IsNewCardPresent()){

    delay(50); 
    return;
  }


  if (!rfid.PICC_ReadCardSerial()){
    return;
  }

  Serial.print("UID tag :");
  for (byte i = 0; i < rfid.uid.size; i++){
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
  lastRead = millis();

}

