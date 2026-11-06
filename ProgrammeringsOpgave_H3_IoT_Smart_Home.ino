#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTC.h"
#include "Termometer.h"
#include <ArduinoJson.h>
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define OLED_RESET -1 // Set to Adurinos buildin reset button

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const unsigned long interval = 3000; 
unsigned long startMillis = millis(); 
unsigned long currentMillis, previousAdjustMillis = 0, previousSelectMillis = 0, previousDownBtnState = 0;
bool showRtc = true, isAdjustMode = false;

// Buttons 
const int selectBtn = 12;
const int upBtn = 8;
const int downBtn = 7;
const int adjustBtn = 3; // aka. adjust time button

DateTime editTime;

// We need this step to handle which time we need to adjust.
int adjustStep = 0; // 0 = hour, 1 = minute, 2 = second
unsigned long previousUpMillis = 0;
unsigned long previousDownMillis = 0;
int previousSelectBtnState = HIGH, previousAdjustBtnState = HIGH, previousUpBtnState = HIGH;
int menu = 0; 

// Those buttons will be handled by the ISR's. 
volatile bool selectBtnPressed = false, upBtnPressed = false, downBtnPressed = false, adjustBtnPressed = false;

const unsigned long buttonDebounce = 20;

// Temperature global vars - Termometer pin and type can be controlled from here. 
uint8_t dhtPin = 2; 
uint8_t dhtType = DHT11;


void setup() {
  Serial.begin(9600);

  // Init Termometer
  initTermometer();

  // Init Rtc
  initRtc();

  // Init display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  // Init control buttons. 
  initButtons();

  // Enables code page 437. Old IBM encoding standard, which allows us to display the celcius degrees symbol.
  display.cp437(true);
}

void initButtons()
{
  // Select Button
  pinMode(selectBtn, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(selectBtn),
    selectBtnIsr,
    CHANGE
  );

    // Up Button
    pinMode(upBtn, INPUT_PULLUP);
    attachInterrupt(
    digitalPinToInterrupt(upBtn),
    upBtnIsr,
    CHANGE
  );

  // Down Button
    pinMode(downBtn, INPUT_PULLUP);
    attachInterrupt(
    digitalPinToInterrupt(downBtn),
    downBtnIsr,
    CHANGE
  );

  // Adjust Button
    pinMode(adjustBtn, INPUT_PULLUP);
    attachInterrupt(
    digitalPinToInterrupt(adjustBtn),
    adjustBtnIsr,
    CHANGE
  );
}


// Whenever the user has verified the Hours, Minutes or the Seconds - They will confirm the number.
// Then it will go to the next step. If we exceeds the adjustStep 3 - Then it will start from the beginning again. 
void calcNextStep()
{
  adjustStep++; 
  if (adjustStep > 3)
    adjustStep = 0;
}

// Down adjustment of the time. 
void downAdjustTime(int step) 
{
  switch(step)
  {
    case 0: 
      editTime = editTime - TimeSpan(0, 1, 0, 0);
    break; 

    case 1: 
      editTime = editTime - TimeSpan(0, 0, 1, 0);
    break; 

    case 2: 
      editTime = editTime - TimeSpan(0, 0, 0, 1);
    break; 
  }
}


// Up adjustment of the time.
void upAdjustTime(int step) 
{
  switch(step)
  {
    case 0: 
      editTime = editTime + TimeSpan(0, 1, 0, 0);
    break; 

    case 1: 
      editTime = editTime + TimeSpan(0, 0, 1, 0);
    break; 

    case 2: 
      editTime = editTime + TimeSpan(0, 0, 0, 1);
    break; 
  }
}


void adjustBtnIsr() 
{
  adjustBtnPressed = true; 
}

void downBtnIsr()
{
  downBtnPressed = true; 
}

void upBtnIsr() 
{
  upBtnPressed = true; 
}

void selectBtnIsr() {
  selectBtnPressed = true;
}




void loop() {
  // Control the Prell from when switching back and forth between the Clock and Temp/RH. 
   if ((millis() - previousSelectMillis) > buttonDebounce && selectBtnPressed) {
    previousSelectMillis = millis();

    selectBtnPressed = false;

    int selectBtnCurrentState = digitalRead(selectBtn);

    if (selectBtnCurrentState == HIGH && previousSelectBtnState == LOW) {
      previousSelectBtnState = HIGH; 
    }
    else if (selectBtnCurrentState == LOW && previousSelectBtnState == HIGH) {
      previousSelectBtnState = LOW;
      if (isAdjustMode)
      {
        calcNextStep();
      }
      else 
      {
        menu = !menu; 
        if (menu == LOW)
          showRtc = true; 
        else 
          showRtc = false; 
      }
    }
  }

  // Adjust Time Prell handling.
  if ((millis() - previousAdjustMillis) > buttonDebounce && adjustBtnPressed)
  {
    previousAdjustMillis = millis();
    adjustBtnPressed = false;

    int adjustBtnCurrentState = digitalRead(adjustBtn);

    if (adjustBtnCurrentState == HIGH && previousAdjustBtnState == LOW)
    {
      previousAdjustBtnState = HIGH;
    }
    else if (adjustBtnCurrentState == LOW && previousAdjustBtnState == HIGH)
    {
      previousAdjustBtnState = LOW;

      isAdjustMode = !isAdjustMode;

      if (isAdjustMode)
      {
        editTime = getDateTime();
        adjustStep = 0;
      }
      else
      {
        // Save the RTC datetime to the RTC variable in the RTC.cpp.
        setDateTime(editTime);
      }
    }
  }

  // Handles the UpButton Prell
  if ((millis() - previousUpMillis) > buttonDebounce && upBtnPressed) {
    previousUpMillis = millis();

    upBtnPressed = false;

    int upBtnCurrentState = digitalRead(upBtn);

    if (upBtnCurrentState == HIGH && previousUpBtnState == LOW) {
      previousUpBtnState = HIGH; 
    }
    else if (upBtnCurrentState == LOW && previousUpBtnState == HIGH) {
      previousUpBtnState = LOW;
      // Do
      upAdjustTime(adjustStep);
    }
  }

  // Handles the Down Button Prell
  if ((millis() - previousDownMillis) > buttonDebounce && downBtnPressed) {
    previousDownMillis = millis();

    downBtnPressed = false;

    int downBtnCurrentState = digitalRead(downBtn);

    if (downBtnCurrentState == HIGH && previousDownBtnState == LOW) {
      previousDownBtnState = HIGH; 
    }
    else if (downBtnCurrentState == LOW && previousDownBtnState == HIGH) {
      previousDownBtnState = LOW;
      // Do
      downAdjustTime(adjustStep);
    }
  }

  // Always show adjustmode as first, if the adjust mode is activated. 
  if (isAdjustMode)
  {
    printAdjustRtc(); 
  }
  else if (showRtc) // Else show RTC as default. 
  {
    printRtc();
    printDate();
  }
  else 
  {
    if (millis() - startMillis >= interval)
    {
        startMillis = millis();
        float humidity = getHumidity();
        float temp = getTemperature();
        printTemperatureAndHumidity(temp, humidity);

        String json = saveMeasurementToJson(temp, humidity);
        Serial.println(json);
    }
  }
}

// Simulate sending json to external rest api.
String saveMeasurementToJson(float temperature, float humidity) 
{
  JsonDocument json; 
  DateTime now = getDateTime(); 
  json["timestamp"] = now.timestamp(DateTime::TIMESTAMP_FULL);
  json["temperature"] = temperature;
  json["humidity"] = humidity; 

  String output; 
  serializeJson(json, output);

  return output;
}

// Print AdjustRtc
void printAdjustRtc()
{
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    display.print(editTime.hour());
    display.print(":");
    display.print(editTime.minute());
    display.print(":");
    display.print(editTime.second());

    display.display();
}

// Print RTC screen
void printRtc() 
{
  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(WHITE); 
  display.setCursor(0,0); 
  String clock = getRtc();
  display.print(clock); 
}

// Display Date
void printDate() 
{
  display.setCursor(0,30); // Start at top-left corner
  String date = getDate();
  display.print(date); 
  display.display();
}

// Display Temperature and RH. 
void printTemperatureAndHumidity(float temp, float humidity) 
{
  display.clearDisplay();
  display.setTextSize(2); 
  display.setTextColor(WHITE); 
  display.setCursor(0,0); 

  String humidityText = "";

  humidityText.concat(F("RH: "));
  humidityText.concat(String(humidity, 0));
  humidityText.concat(F("%"));
  display.print(humidityText);


  String tempText = "";
  display.setCursor(0,30);


  tempText.concat("Temp: ");
  tempText.concat(String(temp, 0));
  tempText.concat(char(248));
  tempText.concat(F("C"));

  display.print(tempText);
  display.display();
}