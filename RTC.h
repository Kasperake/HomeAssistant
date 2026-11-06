#ifndef RTC_H_
#define RTC_H_
#include <Arduino.h>  
#include "RTClib.h"

void initRtc();

String getRtc(); 
String getDate();

DateTime getDateTime();
void setDateTime(DateTime dateTime);

#endif RTC_H_
