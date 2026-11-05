#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTC.h"
#include "Termometer.h"
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned long interval = 3000; 
unsigned long startMillis = millis(); 
unsigned long currentMillis, previousAdjustMillis = 0, previousSelectMillis = 0, previousDownBtnState = 0;
bool showRtc = true, isAdjustMode = false;
// Menu 
const int selectBtn = 12;
const int upBtn = 8;
const int downBtn = 7;
const int adjustBtn = 3; // aka. Indstil tid menu

DateTime editTime;
int adjustStep = 0; // 0 = hour, 1 = minute, 2 = second

unsigned long previousUpMillis = 0;
unsigned long previousDownMillis = 0;

int previousSelectBtnState = HIGH, previousAdjustBtnState = HIGH, previousUpBtnState = HIGH;
int menu = 0; 
volatile bool selectBtnPressed = false, upBtnPressed = false, downBtnPressed = false, adjustBtnPressed = false;
const unsigned long buttonDebounce = 20;

// Temperature global vars 
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

  initButtons();

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


void calcNextStep()
{
  adjustStep++; 
  if (adjustStep > 3)
    adjustStep = 0;
}

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
  // Menu: RTC - Temp/RH
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

// Adjust Time 
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

  // UpBtn press
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

  // Down button press
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

  if (isAdjustMode)
  {
    printAdjustRtc(); 
  }
  else if (showRtc)
  {
    printRtc();
    printDate();
  }
  else 
  {
    printDht();
  }
}

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

void printRtc() 
{
  display.clearDisplay();
  display.setTextSize(2); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner
  String clock = getRtc();
  display.print(clock); 
}

void printDate() 
{
  display.setCursor(0,30); // Start at top-left corner
  String date = getDate();
  display.print(date); 
  display.display();
}

void printDht() 
{
  display.clearDisplay();
  display.setTextSize(2); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner

  String humidity = getHumidity();
  display.print(humidity);

  display.setCursor(0,30);
  String temp = getTemperature();
  display.print(temp);
  display.display();
}