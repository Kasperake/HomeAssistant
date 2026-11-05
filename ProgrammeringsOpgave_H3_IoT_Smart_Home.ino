#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include "RTClib.h"
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
#define DHTPIN 2 
#define DHTTYPE DHT11
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
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


void setup() {
  Serial.begin(9600);

  // Init Termometer
  dht.begin();

  // Init display OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  
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
    printRtc();
  else
    printDht();
}

void printRtc() 
{
  DateTime now = rtc.now(); 
  display.clearDisplay();
  display.setTextSize(2); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner
  
  if (now.hour() < 10)
    display.print("0");
  display.print(now.hour(), DEC);
  display.print(':');
  
  if (now.minute() < 10)
    display.print("0");
  display.print(now.minute(), DEC);
  display.print(':');
  if (now.second() < 10)
    display.print("0");
  display.print(now.second(), DEC);



  display.setCursor(0,30); // Start at top-left corner
  display.print(now.day(), DEC);
  display.print('.');
  display.print(now.month(), DEC);
  display.print('.');
  display.print(now.year(), DEC);
  display.display();
}
void printDht() 
{
  display.clearDisplay();
  display.setTextSize(2); // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0,0); // Start at top-left corner
    // Wait a few seconds between measurements.
  //delay(2000);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  display.print(F("RH: "));
  display.print(h);
  display.print(F("%"));
  display.setCursor(0,30);
  display.print("Temp: ");
  display.print(t, 0);
  display.cp437(true);
  display.write(248); // °
  display.print(F("C"));
  display.display();
}