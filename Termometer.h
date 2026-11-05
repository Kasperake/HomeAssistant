#ifndef TERMOMETER_H_
#define TERMOMETER_H_
#include <Arduino.h>  // includer nødvendige .h-filer
#include "DHT.h"

extern uint8_t dhtPin;
extern uint8_t dhtType;

void initTermometer();

String getTemperature(); 
String getHumidity();

#endif TERMOMETER_H_
