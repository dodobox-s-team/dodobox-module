#include <DHT.h>
#include <ESP8266WiFi.h>

#define DHTTYPE DHT11

//const char* ssid = "Proximus-Home-0548";
//const char* password = "wpfx6kjkux623";


const int DHTPin = 5;

DHT dht(DHTPin, DHTTYPE);

static char celsiusTemp[7];
static char humidityTemp[7];

void setup()
{
  Serial.begin(9600);
  delay(10);
  dht.begin();
  delay(10000);
}

void loop()
{
  if (true) 
  {
    while (true) 
    {
          float h = dht.readHumidity();
          float t = dht.readTemperature();
          if (isnan(h) || isnan(t))
          {
            Serial.println("Failed to read from DHT sensor!");
            strcpy(celsiusTemp,"Failed");
            strcpy(humidityTemp, "Failed");
          }
          else
          {
            float hic = dht.computeHeatIndex(t, h, false);
            dtostrf(hic, 6, 2, celsiusTemp);
            dtostrf(h, 6, 2, humidityTemp);
          }
          Serial.println(celsiusTemp);
          delay(5000);          
          break;
        }
        }
}