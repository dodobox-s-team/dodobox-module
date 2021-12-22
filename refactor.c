#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <DHT.h>

#define DHTTYPE DHT11

// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x6A, 0xED, 0x3D, 0x4C, 0x32, 0xAA, 0x70, 0xF9, 0x9E, 0x25, 0x59, 0xAF, 0xDD, 0x0C, 0x27, 0x7C, 0x11, 0x68, 0x89, 0x0C};
const char* remote_hostess = "google.com";
const char* remote_host = "severin.dodobox.site";
const char* METHOD_GET = "GET";
const char* METHOD_POST = "POST";
const char* METHOD_PUT = "PUT";

bool TEMPERATURE = false;
bool HUMIDITY = false;
bool AIR_QUALITY = false;
int ID_TEMPERATURE;
int ID_HUMIDITY;
int ID_AIR_QUALITY;

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
  WiFi.begin("WIFI - GREEN", "AE00968A1D");
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
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {
    // Récupérer les informations sur les modules de notre esp
    // Faire le chemin de la requête
    char cible[150];
    String cibleString = "https://severin.dodobox.site/api/devices/ip/" + WiFi.localIP().toString();
    cibleString.toCharArray(cible, 150);
    Serial.println(cible);

    // Faire la requête
    DynamicJsonDocument devices = httpRequest(METHOD_GET, cible, "");
    
    for(int i=0; i < devices.size(); i++) {
      if(devices[i]["type"]==1) {
        TEMPERATURE = devices[i]["toggle"];
        ID_TEMPERATURE = devices[i]["id"];
      } else if(devices[i]["type"]==2) {
        HUMIDITY = devices[i]["toggle"];
        ID_HUMIDITY = devices[i]["id"];
      }else if(devices[i]["type"]==3) {
        AIR_QUALITY = devices[i]["toggle"];
        ID_AIR_QUALITY = devices[i]["id"];
      }
    }
    

    if(TEMPERATURE) {
      Serial.println("temperature");
      float temperature = dht.readTemperature();
      if (isnan(temperature)) {
        Serial.println("Failed to read temperature from DHT sensor!");
        sprintf(cible, "https://severin.dodobox.site/api/devices/%d/false", ID_TEMPERATURE);
        DynamicJsonDocument devices = httpRequest(METHOD_PUT, cible, "");
      } else {
        Serial.printf("%f °C");
        
      }
    }
    if(HUMIDITY) {
      Serial.println("humidity");
      float humidity = dht.readHumidity();
      if (isnan(humidity)) {
        Serial.println("Failed to read humidity from DHT sensor!");
        sprintf(cible, "https://severin.dodobox.site/api/devices/%d/false", ID_HUMIDITY);
        DynamicJsonDocument devices = httpRequest(METHOD_PUT, cible, "");
      } else {
        Serial.printf("%f %");
        
      }
    }
    if(AIR_QUALITY) {
      Serial.println("quality");
      quality = analogRead(MQ6_PIN);
      if (isnan(quality)) {
        Serial.println("Failed to read air quality from MQ-6 sensor!");
        
      } else {
        Serial.println(quality);
        
      }
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(30000);
}

DynamicJsonDocument httpRequest(const char methode[], char cible[], char data[]) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  client->setFingerprint(fingerprint);
  HTTPClient https;

  Serial.print("[HTTPS] begin...\n");
  if (https.begin(*client, cible)) {  // HTTPS
    Serial.printf("[HTTPS] %s...\n", methode);

    // Faire la requête GET ou POST
    int httpCode;
    if(methode == METHOD_PUT) {
      httpCode = https.sendRequest("PUT", data);
    } else { 
      httpCode = (methode == METHOD_POST) ? https.POST(data) : https.GET();
    }
    
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] %s... code: %d\n", methode, httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        //JsonObject[] response = dataStringToJsonList(payload);
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
        }
        //JsonArray response = doc.as<JsonArray>();
        
        return doc;
      }
    } else {
      Serial.printf("[HTTPS] %s... failed, error: %s\n", methode, https.errorToString(httpCode).c_str());
    }

    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
}