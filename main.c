#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Process.h>

#define DHTTYPE DHT11

void sendData(str url);

// Pin of all modules
// VCC -> 3v; Signal -> D1; GND -> G
const int DS18B20_PIN = 4;
const int MQ6_PIN = 'A0';
const int DHT_PIN = 'D1';

// boolean to active modules
bool air_quality = true;
bool temp = true;
bool temp_humidity = true;

DHT dht(DHTPin, DHTTYPE);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

void setup() {
  // Start the Serial Monitor
  Serial.begin(9600);
  // Start the DS18B20 sensor
  sensors.begin();
  sendData('https://172.17.14.33/api/devices')
}

void loop() {
  if(temp_humidity)
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
  }
  else if(temp)
  {
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
  }
  if(air_quality)
  {
    air = analogRead(MQ6_PIN);
    Serial.println(air);
  }

  
  delay(60000);
}

void sendData(str url) {
    Process p;
  p.begin("curl");
  p.addParameter('-k');
  p.addParameter(url);
  p.addParameter('-X');
  p.addParameter('GET')
  p.addParameter('-H')
  p.addParameter('accept: application/json')
  
  p.run();
  while (p.available()>0) {
    char c = p.read();
    Serial.print(c);
  }
  // Ensure the last bit of data is sent.
  Serial.flush(); 
}