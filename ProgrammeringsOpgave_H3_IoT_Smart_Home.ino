#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTC.h"
#include "Termometer.h"
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const unsigned long interval = 3000; 
unsigned long startMillis = millis(); 
unsigned long currentMillis, previousMillis;
bool showRtc = true;

// Menu 
const int menuBtn = 12;
int previousState = HIGH;
int menu = 0;
unsigned long previousPress = 0;
volatile bool menuButtonPressed = false;
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

  // Menu Button
  pinMode(menuBtn, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(menuBtn),
    menuButtonIsr,
    CHANGE
  );
}

void menuButtonIsr() {
  menuButtonPressed = true;
}

void loop() {
  // Handle Menu button prell.
  if ((millis() - previousPress) > buttonDebounce && menuButtonPressed) {
    previousPress = millis();

    menuButtonPressed = false;

    int currentState = digitalRead(menuBtn);

    if (currentState == LOW && previousState == HIGH) {
      previousState = LOW; 
    }
    else if (currentState == HIGH && previousState == LOW) {
      previousState = HIGH;
      menu = !menu; 
      if (menu == HIGH)
        showRtc = true;
      else 
        showRtc = false;
    }
  }

  if (showRtc)
  {
    printRtc();
    printDate();
  }
  else
  {
    printDht();
  }
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