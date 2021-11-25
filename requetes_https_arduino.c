#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266Ping.h> //add ESP8266Ping.zip library manually url:https://github.com/dancol90/ESP8266Ping
#include <ESP8266HTTPClient.h>

#include <WiFiClientSecureBearSSL.h>
// !! Fingerprint need to be changed every time docker compose is rebuild !!
const uint8_t fingerprint[20] = {0x21, 0xc6, 0xb0, 0x56, 0x9a, 0x3c, 0x35, 0xbc, 0xcd, 0xf4, 0x60, 0x8d, 0xd7, 0x4b, 0x83, 0x1a, 0x43, 0x19, 0x13, 0xb0};
const IPAddress remote_ip(192 ,168 , 1, 36);

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
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  //Pinging
  Serial.print("Pinging ip ");
  Serial.println(remote_ip);

  if(Ping.ping(remote_ip)){
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
    if (https.begin(*client, "https://192.168.1.36/api/devices")) {  // HTTPS

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