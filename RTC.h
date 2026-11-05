#ifndef RTC_H_
#define RTC_H_
#include <Arduino.h>  // includer nødvendige .h-filer
#include "RTClib.h"

void initRtc();

String getRtc(); 
String getDate();

#endif RTC_H_
