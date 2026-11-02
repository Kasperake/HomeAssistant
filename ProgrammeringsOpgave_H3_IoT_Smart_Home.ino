int menuBtn = 12; 

volatile int menuBtnState = HIGH; // W. PULLUP REVERSE

void menuButtonIsr() 
{
  menuBtnState = digitalRead(menuBtn);
}



void setup() {
  Serial.begin(9600);

  pinMode(menuBtn, INPUT_PULLUP);
  attachInterrupt(
    digitalPinToInterrupt(menuBtn),
    menuButtonIsr,
    CHANGE
  );
}


void loop() {
  Serial.println(menuBtnState);
}
