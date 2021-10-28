#include <DHT.h>
#include <ESP8266WiFi.h>

#define DHTTYPE DHT11

const char* ssid = "Esp";
const char* password = "dodoboxx";

WiFiServer server(80);


const int DHTPin = 5;

DHT dht(DHTPin, DHTTYPE);

static char celsiusTemp[7];
static char humidityTemp[7];

void setup()
{
  Serial.begin(9600);
  delay(10);
  dht.begin();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  Serial.println("setup finished");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);

  Serial.println(WiFi.localIP());
}

void loop()
{
  WiFiClient client = server.available();

  if (client) 
  {
    Serial.println("New client");
    boolean blank_line = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        if (c == '\n' && blank_line) 
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
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          
          client.println(celsiusTemp);
          client.println(humidityTemp);
          break;
        }
        if (c == '\n') 
        {
          blank_line = true;
        }
        else if (c != '\r') 
        {
          blank_line = false;
        }
      }
    }
    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}