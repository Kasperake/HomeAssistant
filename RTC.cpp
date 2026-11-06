#include "RTC.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void initRtc() 
{
    // Init RTC
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
  }

  // RTC Adjust
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}


String getRtc() 
{
    DateTime now = rtc.now(); 

    String text = "";
  
  // Hour
  if (now.hour() < 10)
    text.concat("0");
  text.concat(now.hour());
  text.concat(":");
  
  // Minute
  if (now.minute() < 10)
    text.concat("0");
  text.concat(now.minute());
  text.concat(":");

  // Second
  if (now.second() < 10)
    text.concat("0");
  text.concat(now.second());

  return text;
}


String getDate()
{
  DateTime now = rtc.now(); 
  String text = "";

  text.concat(now.day());
  text.concat(".");
  text.concat(now.month());
  text.concat(".");
  text.concat(now.year());
  
  return text; 
}

DateTime getDateTime() 
{
  return rtc.now();
}

void setDateTime(DateTime dateTime)
{
  rtc.adjust(dateTime);
}
