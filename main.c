#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#define DHTTYPE DHT11

// Pin of all modules
const int DS18B20_PIN = 4;
const int MQ6_PIN = 'A0';
const int DHT_PIN = 5;

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