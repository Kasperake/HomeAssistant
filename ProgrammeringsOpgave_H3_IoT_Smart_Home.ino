const int menuBtn = 12;

int previousState = HIGH;
int menu = 0;
unsigned long previousPress = 0;
volatile bool menuButtonPressed = false;
const unsigned long buttonDebounce = 20;

void setup() {
  Serial.begin(9600);
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
        Serial.print("Clock");
      else 
        Serial.print("Temperature");
    }
  }
}