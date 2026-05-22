#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "../lib/includes.h"
#include "../lib/config.h"

MFRC522 rfid(SS_PIN, RST_PIN); // Create an instance of the MFRC522 class
unsigned long lastRead = 0;
unsigned long lastUidTime = 0;  // Timestamp de la última lectura
String lastUid = "";             // UID de la última lectura
bool wifiConnected = false;
const unsigned long RESET_INTERVAL = READER_RESET_INTERVAL;
const unsigned long UID_DEBOUNCE_MS = 5000;  // 5 segundos entre lecturas del mismo UID

void printSeparator() {
  Serial.println("========================================");
}

void connectWifi(){
  printSeparator();
  Serial.println("Conectando a WiFi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    printSeparator();
    Serial.println("WiFi conectado!");
    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway (backend): ");
    Serial.println(API_HOST);
    printSeparator();
  } else {
      Serial.println("\nError: No se pudo conectar a WiFi");
  }


}

void sendAttendance(String uid){
  Serial.print("WiFi status: ");
  Serial.println(WiFi.status());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.print("Gateway IP: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("Subnet: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("DNS: ");
  Serial.println(WiFi.dnsIP());

  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi no conectado. No se puede enviar asistencia.");
    return;
  }

  String url = "http://" + String(API_HOST) + ":" + String(API_PORT) + API_BASE_PATH;

  Serial.print("Enviando a: ");
  Serial.println(url);

  String body = "{\"rfid_uuid\":\"" + uid + "\"}";

  Serial.print("Body: ");
  Serial.println(body);

  
  

  int httpCode = -1;
  int retries = 3;

  while (httpCode == -1 && retries > 0) {
    HTTPClient http;
    http.setTimeout(20000);
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", API_COMPANY);

    httpCode = http.POST(body);

    if (httpCode == -1 && retries > 1) {
      Serial.println("Conexion fallida, reintentando...");
      retries--;
      delay(2000);
    } else {
      retries = 0;
    }

    if (httpCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpCode);

      if (httpCode == 200 || httpCode == 201){
        String payload = http.getString();
        Serial.print("Respuesta servidor: ");
        Serial.println(payload);
      }
      else {
        Serial.print("Error HTTP: ");
        Serial.println(httpCode);
      }
    }

    http.end();
  }

  if (httpCode == -1) {
    Serial.println("Error: No se pudo conectar al servidor");
  }

}

void setup() {
  Serial.begin(115200); // Initialize serial communication
  Serial.println("Sistema inicianod - Gestion de turnos");
  SPI.begin(); // Initialize SPI bus
  rfid.PCD_Init(); // Initialize the MFRC522 RFID reader
  connectWifi();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() != WL_CONNECTED){
    if (wifiConnected){
      Serial.println("Wifi desconectado. Reconectando... ");
    }
    connectWifi();
  }

  if (millis() - lastRead > READER_RESET_INTERVAL) {
    rfid.PCD_Init();
    delay(100);
    lastRead = millis();
  }

  if (!rfid.PICC_IsNewCardPresent()){

    delay(RFID_POLL_INTERVAL); 
    return;
  }


  if (!rfid.PICC_ReadCardSerial()){
    return;
  }

  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++){
    if (i> 0) uidStr += ":";
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }

  Serial.print("Tarjeta detectada - UID: ");
  Serial.println(uidStr);

  // Debounce: ignorar si es la misma tarjeta dentro de UID_DEBOUNCE_MS
  unsigned long now = millis();
  if (uidStr == lastUid && (now - lastUidTime) < UID_DEBOUNCE_MS) {
    Serial.print("Ignorando lectura duplicada (espera ");
    Serial.print(UID_DEBOUNCE_MS / 1000);
    Serial.println("s entre lecturas)");
    lastUidTime = now; // Extender el debounce
    delay(RFID_POLL_INTERVAL);
    return;
  }

  lastUid = uidStr;
  lastUidTime = now;

  sendAttendance(uidStr);

  // Resetear el lector RFID para evitar lecturas fantasma de la misma tarjeta
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  // Esperar hasta que la tarjeta sea retirada
  unsigned long waitStart = millis();
  while (rfid.PICC_IsNewCardPresent() && (millis() - waitStart) < 3000) {
    delay(100);
  }

  lastRead = millis();

}

