#include "Termometer.h"

DHT dht(dhtPin, dhtType);

void initTermometer()
{
    // Init Termometer
  dht.begin();
}

String getTemperature() 
{
  String text = "";

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    text = F("Failed to read the temperature from DHT sensor!");
    Serial.println(text);
    return text;
  }
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  text.concat("Temp: ");
  text.concat(String(t, 0));
  text.concat(char(248));
  text.concat(F("C"));

  return text; 
}


String getHumidity() 
{
  String text = "";
  float h = dht.readHumidity();

  if (isnan(h)) {
    text = F("Failed to read the humidity from DHT sensor!");
    Serial.println(text);
    return text;
  }


  text.concat(F("RH: "));
  text.concat(String(h, 0));
  text.concat(F("%"));

  return text; 
}