//Llibreries
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include <ArduinoJson.h>

#include "certs.h"

#ifndef STASSID
#define STASSID "SercommC3B0"
#define STAPSK "4HTTCK5LJFNJ9Q"
#endif

#include <Adafruit_NeoPixel.h>

ESP8266WiFiMulti WiFiMulti;

//CIRCUIT
const int ledPin = 1;
const int pixels = 27; //Cada led equival a 50 persones
const int vibrar = 5;
Adafruit_NeoPixel tiraLeds(pixels, ledPin, NEO_GRB + NEO_KHZ800);
int numLeds = 0; //Nombre de leds que es mostren a la polsera

int dadaIndex = 0; //Index de la base de dades

void setup() {
  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(STASSID, STAPSK);
  Serial.println("setup() done connecting to ssid '" STASSID "'");

  delay(500);

  //CIRCUIT
  pinMode(vibrar, OUTPUT);
  tiraLeds.begin();
  //Per defecte tots els leds estan apagats
  for (int ind = 0; ind < pixels; ind++) {
    tiraLeds.setPixelColor(ind, tiraLeds.Color(0, 0, 0));
    tiraLeds.show();
  }
  delay(500);
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClientSecure client;
    client.setInsecure();
    //RESET: Si s'arriba a la fila 42 (ja no existeix), es torna a l'inici
    if(dadaIndex == 42){
      dadaIndex = 0;
    }
    //HTTPS
    String fullUrl = jigsaw_host + String(dadaIndex) + ".json";
    dadaIndex++; //Sumar per canviar fila base de dades
    Serial.print("URL:"); Serial.println(fullUrl);

    client.connect(fullUrl, jigsaw_port);

    HTTPClient https;

    Serial.print("[HTTPS] begin X ...\n");
    if (https.begin(client, fullUrl)) {  // HTTPS

      https.addHeader("Content-Type", "application/json");
      Serial.print("[HTTPS] GET...\n");
      // Start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);

          //PROCESSAR DADES
          // JSON data
          DynamicJsonDocument doc(1024);
          deserializeJson(doc, payload);

          // Accedir als valors del JSON
          JsonObject obj = doc[0];
          const char* id = obj["id"];
          const char* number_str = obj["number"];
          int number = atoi(number_str); // Convertir l'string en un int per poder-ho dividir
          numLeds = number / 50;
          const char* types = obj["types"];
          const char* date = obj["date"];
          
          // Printar els valors
          Serial.println("Parsed JSON data:");
          Serial.print("ID: "); Serial.println(id);
          Serial.print("Number: "); Serial.println(number);
          Serial.print("NumLeds: "); Serial.println(numLeds);
          Serial.print("Types: "); Serial.println(types);
          Serial.print("Date: "); Serial.println(date);

          //ACCIONS (leds + vibració)
          if(strcmp(types, "rescat") == 0){ //Si és un rescat...
            vermellInicial(); //S'inicialitzen els leds (es mostren de manera fixe)
            SOS(); //Vibració
            vermell(); //Els leds comencen a córrer
            delay(5000);
            reset(); //S'apaguen (estat per defecte de la polsera)
            delay(5000);
          } else if(strcmp(types, "naufragi") == 0){ //Si és un naufragi...
            blancInicial();
            cor();
            blanc();
            delay(5000);
            reset();
            delay(5000);
          } else if(strcmp(types, "parats") == 0){ //Si el vaixell està parat (immobilitzat a port)
            sequencia();
            reset();
            delay(5000);
          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end();
      numLeds = 0;
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }
  Serial.println("Wait 10s before next round...");
  delay(10000);
}

void blancInicial() {
  for (int ind = 0; ind <= numLeds; ind++) {
    tiraLeds.setPixelColor(ind, tiraLeds.Color(255, 255, 255));
    tiraLeds.show();
  }
}
void blanc() {
  int repetir = 3;
  int grup = numLeds;
  for (int i = 0; i < repetir; i++){
    for (int i = 0; i < pixels - numLeds + 1; i++) {
      // Que es pinti vermell
      for (int p = 0; p < numLeds; p++) {
        tiraLeds.setPixelColor(i + p, tiraLeds.Color(255, 255, 255));
      }
      // Ensenyar-ho
      tiraLeds.show();
      delay(100);
      // Que s'apaguin
      for (int p = 0; p < numLeds; p++) {
        tiraLeds.setPixelColor(i + p, 0);
      }
    }
  }
}

void vermellInicial() {
  for (int ind = 0; ind <= numLeds; ind++) {
    tiraLeds.setPixelColor(ind, tiraLeds.Color(255, 0, 0));
    tiraLeds.show();
  }
}
void vermell() {
  int repetir = 3;
  int grup = numLeds;
  for (int i = 0; i < repetir; i++){
    for (int i = 0; i < pixels - numLeds + 1; i++) {
      // Que es pinti vermell
      for (int p = 0; p < numLeds; p++) {
        tiraLeds.setPixelColor(i + p, tiraLeds.Color(255, 0, 0));
      }
      // Ensenyar-ho
      tiraLeds.show();
      delay(100);
      // Que s'apaguin
      for (int p = 0; p < numLeds; p++) {
        tiraLeds.setPixelColor(i + p, 0);
      }
    }
  }
}

void sequencia() {
  int repetir = 5;
  int grup = 3;
  for (int i = 0; i < repetir; i++){
    for (int i = 0; i < pixels - grup + 1; i++) {
      // Que es pinti groc
      for (int p = 0; p < grup; p++) {
        tiraLeds.setPixelColor(i + p, tiraLeds.Color(255, 233, 0));
      }
      // Ensenyar-ho
      tiraLeds.show();
      delay(100);
      // Que s'apaguin
      for (int p = 0; p < grup; p++) {
        tiraLeds.setPixelColor(i + p, 0);
      }
      digitalWrite(vibrar,HIGH);
      delay(100);
      digitalWrite(vibrar,LOW);
    }
  }
}

void cor(){
  for (int comptador = 0; comptador <= 2; comptador++){
    digitalWrite(vibrar,HIGH);
    delay(100);
    digitalWrite(vibrar,LOW);
    delay(100);
    digitalWrite(vibrar,HIGH);
    delay(100);
    digitalWrite(vibrar,LOW);
    delay(750);
  }
  delay(250);
  digitalWrite(vibrar,HIGH);
  delay(100);
  digitalWrite(vibrar,LOW);
  delay(250);
  digitalWrite(vibrar,HIGH);
  delay(100);
  digitalWrite(vibrar,LOW);
}

void SOS(){
  int comptador = 0;
  int temps = 0;

  for (comptador = 0; comptador <= 4; comptador++){
    while(temps < 150){
      digitalWrite(vibrar,HIGH);
      delay(1);
      digitalWrite(vibrar,LOW);
      delay(1);
      temps = temps +2;
    }
    comptador++;
    temps = 0;
    delay(100);
  }
  comptador = 0;
  delay(200);

  for (comptador = 0; comptador <= 4; comptador++){
    while(temps < 300){
      digitalWrite(vibrar,HIGH);
      delay(1);
      digitalWrite(vibrar,LOW);
      delay(1);
      temps = temps +2;
    }
    comptador++;
    temps = 0;
    delay(100);
  }
  comptador = 0;
  delay(200);

  for (comptador = 0; comptador <= 4; comptador++){
    while(temps < 150){
      digitalWrite(vibrar,HIGH);
      delay(1);
      digitalWrite(vibrar,LOW);
      delay(1);
      temps = temps +2;
    }
    comptador++;
    temps = 0;
    delay(100);
  }
  comptador = 0;
  delay(2000);
}

void reset(){
  for (int ind = 0; ind <= pixels; ind++) {
    tiraLeds.setPixelColor(ind, tiraLeds.Color(0, 0, 0));
    tiraLeds.show();
  }
}