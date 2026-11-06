#include "Termometer.h"

DHT dht(dhtPin, dhtType);

void initTermometer()
{
    // Init Termometer
  dht.begin();
}

float getTemperature() 
{
  String text = "";

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    text = F("Failed to read the temperature from DHT sensor!");
    Serial.println(text);
    return 0;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);


  return hic; 
}


float getHumidity() 
{
  float h = dht.readHumidity();

  String text = "";

  if (isnan(h)) {
    text = F("Failed to read the humidity from DHT sensor!");
    Serial.println(text);
    return 0;
  }

  return h; 
}