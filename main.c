#include "credentials.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DHT.h>

#define DHTTYPE DHT11

const char* ssid = mySSID;
const char* password = myPASSWORD;
// Fingerprint for Https
const uint8_t fingerprint[20] = {0xEA, 0x19, 0x44, 0x33, 0x74, 0x4E, 0x7F, 0x09, 0xE6, 0xF3, 0xDD, 0x5E, 0x9A, 0x23, 0xB7, 0x13, 0x4F, 0x38, 0x7A, 0xE4};
const char* METHOD_GET = "GET";
const char* METHOD_POST = "POST";
const char* METHOD_PUT = "PUT";

bool TEMPERATURE = false;
bool HUMIDITY = false;
bool AIR_QUALITY = false;
int ID_TEMPERATURE;
int ID_HUMIDITY;
int ID_AIR_QUALITY;
int TGraphId;
int HGraphId;
int QGraphId;

IPAddress staticIP(192,168,1,99);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8,8,8,8);

const int MQ6_PIN = 'A0';
const int DHT_PIN = 'D1';
DHT dht(DHT_PIN, DHTTYPE);
float temperature;
float humidity;
int quality;

void setup() {

  Serial.begin(9600);
  Serial.setDebugOutput(true);

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi network with IP Address: ");
  Serial.print("Mon adresse IP");
  Serial.println(WiFi.localIP());
  WiFi.config(staticIP, gateway, subnet, dns);
  Serial.print("Ma nouvelle adresse IP");
  Serial.println(WiFi.localIP());
  

}


void loop() {
  // Vérifie à chaque fois qu'il est connecté au WiFi
  if ((WiFi.status() == WL_CONNECTED)) {
    // Récupérer les informations sur les modules de notre esp grâce à son IP
    // Faire le chemin de la requête
    char cible[150];
    String cibleString = "https://severin.dodobox.site/api/devices/ip/" + WiFi.localIP().toString();
    cibleString.toCharArray(cible, 150);
    Serial.println(cible);

    // Faire la requête
    DynamicJsonDocument devices = httpRequest(METHOD_GET, cible, "");

    // Passer dans chaque module pour vérifier s'il est activé
    for(int i=0; i < devices.size(); i++) {
      if(devices[i]["type"]==1) {
        TGraphId = getGraphId(devices[i]["id"]);
        TEMPERATURE = devices[i]["toggle"];
        ID_TEMPERATURE = devices[i]["id"];
      } else if(devices[i]["type"]==2) {
        HGraphId = getGraphId(devices[i]["id"]);
        HUMIDITY = devices[i]["toggle"];
        ID_HUMIDITY = devices[i]["id"];
      }else if(devices[i]["type"]==3) {
        QGraphId = getGraphId(devices[i]["id"]);
        AIR_QUALITY = devices[i]["toggle"];
        ID_AIR_QUALITY = devices[i]["id"];
      }
    }

    Serial.printf("TGraphId : %d, TEMPERATURE : %s, ID_TEMPERATURE : %d\n", TGraphId, TEMPERATURE ? "true" : "false", ID_TEMPERATURE);
    Serial.printf("HGraphId : %d, HUMIDITY : %s, ID_HUMIDITY : %d\n", HGraphId, HUMIDITY ? "true" : "false", ID_HUMIDITY);
    Serial.printf("QGraphId : %d, AIR_QUALITY : %s, ID_AIR_QUALITY : %d\n", QGraphId, AIR_QUALITY ? "true" : "false", ID_AIR_QUALITY);

    char data[70];
    // Passer dans chaque module activer pour récolter les données et les envoyer
    if(TEMPERATURE) {
      Serial.println("temperature");
      float temperature = dht.readTemperature();                                            // Lire la température
      if (isnan(temperature)) {                                                             // Si le module est absent ou défaillant, le désactiver dans la DB
        Serial.println("Failed to read temperature from DHT sensor!");
        sprintf(cible, "https://severin.dodobox.site/api/devices/%d/false", ID_TEMPERATURE);
        httpRequest(METHOD_PUT, cible, "");
      } else {
        Serial.printf("%f °C");
        sprintf(data, "{\"id\": 0\"time\": \"2021-12-22T19:32:02.811Z\",\"graphId\": %d,\"value\": %d}", TGraphId, temperature);
        Serial.println(data);
        httpRequest(METHOD_POST, "https://severin.dodobox.site/api/graphData", data);           // Envoyer les données
      }
    }
    if(HUMIDITY) {
      Serial.println("humidity");
      float humidity = dht.readHumidity();                                                // Lire l'humidité
      if (isnan(humidity)) {                                                              // Si le module est absent ou défaillant, le désactiver dans la DB
        Serial.println("Failed to read humidity from DHT sensor!");
        sprintf(cible, "https://severin.dodobox.site/api/devices/%d/false", ID_HUMIDITY);
        httpRequest(METHOD_PUT, cible, "");
      } else {
        Serial.printf("%f %");
        sprintf(data, "{\"id\": 0\"time\": \"2021-12-22T19:32:02.811Z\",\"graphId\": %d,\"value\": %d}", HGraphId, humidity);
        Serial.println(data);
        httpRequest(METHOD_POST, "https://severin.dodobox.site/api/graphData", data);           // Envoyer les données
      }
    }
    if(AIR_QUALITY) {
      Serial.println("quality");
      quality = analogRead(MQ6_PIN);                                                      // Lire la qualité de l'air
      if (isnan(quality)) {                                                               // Si le module est absent ou défaillant, le désactiver dans la DB
        Serial.println("Failed to read air quality from MQ-6 sensor!");
        sprintf(cible, "https://severin.dodobox.site/api/devices/%d/false", ID_AIR_QUALITY);
        httpRequest(METHOD_PUT, cible, "");
      } else {
        Serial.println(quality);
        sprintf(data, "{\"id\": 0\"time\": \"2021-12-22T19:32:02.811Z\",\"graphId\": %d,\"value\": %d}", QGraphId, quality);
        Serial.println(data);
        httpRequest(METHOD_POST, "https://severin.dodobox.site/api/graphData", data);           // Envoyer les données
      }
    }
  }

  Serial.println("Wait 60s before next round...");
  delay(60000);
}

int getGraphId(int id) {
  DynamicJsonDocument graphs = httpRequest(METHOD_GET, "https://severin.dodobox.site/api/graph", "");

  // Passer dans chaque graph pour trouver le bon
  for(int i=0; i < graphs.size(); i++) {
    int deviceId = graphs[i]["deviceId"];
    int graphId = graphs[i]["id"];
    Serial.printf("deviceId: %d, graphId: %d, id: %d\n", deviceId, graphId, id);
    if(deviceId==id) {
      Serial.println("graphId is ok");
      return graphId;
    }
  }
  return 0;
}

DynamicJsonDocument httpRequest(const char methode[], char cible[], char data[]) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  client->setFingerprint(fingerprint);
  HTTPClient https;

  Serial.print("[HTTPS] begin...\n");
  if (https.begin(*client, cible)) {
    Serial.printf("[HTTPS] %s...\n", methode);

    // Faire la requête GET ou POST
    int httpCode;
    if(methode == METHOD_PUT) {
      httpCode = https.sendRequest("PUT", data);
    } else { 
      httpCode = (methode == METHOD_POST) ? https.POST(data) : https.GET();
    }

    // httpCode est negatif en cas d'erreur
    if (httpCode > 0) {
      // HTTP header est envoyé
      Serial.printf("[HTTPS] %s... code: %d\n", methode, httpCode);

      // Fichier trouvé au serveur
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        if(error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        return doc;
      }
    } else {
      Serial.printf("[HTTPS] %s... failed, error: %d %s\n", methode, httpCode, https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
}