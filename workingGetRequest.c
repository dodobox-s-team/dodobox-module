#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266Ping.h> //add ESP8266Ping.zip library manually
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>


// Fingerprint for demo URL, expires on June 2, 2021, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x16, 0x5b, 0x8d, 0x1c, 0x08, 0xf8, 0x7a, 0xaa, 0xc9, 0x3b, 0x1e, 0x42, 0x0c, 0x80, 0x23, 0x22, 0xc5, 0xea, 0x36, 0x38};
const char* remote_hostess = "google.com";
const char* remote_host = "quentin.dodobox.site";
IPAddress staticIP(192,168,1,100);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8,8,8,8);

void setup() {

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin("Proximus-Home-0548", "wpfx6kjkux623");
  //WiFi.begin("huawei cycy", "wow159357");
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.print("Mon adresse IP");
  Serial.println(WiFi.localIP());
  WiFi.config(staticIP, gateway, subnet, dns);
  Serial.print("Ma nouvelle adresse IP");
  Serial.println(WiFi.localIP());

  //Pinging
  Serial.print("Pinging ip ");
  Serial.println(remote_hostess);
  if(Ping.ping(remote_hostess)){
    Serial.println("Success!!");
    }else{
      Serial.println("Error");
      }
  Serial.println(remote_host);
  if(Ping.ping(remote_host)){
    Serial.println("Success!!");
    }else{
      Serial.println("Error");
      }
}


void loop() {
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    // client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://quentin.dodobox.site/api/devices")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = https.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println("Wait 10s before next round...");
  delay(10000);
}