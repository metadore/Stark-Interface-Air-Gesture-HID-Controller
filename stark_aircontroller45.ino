#include <Mouse.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <LedControl.h>

// --- PIN CONFIGURATION ---
const int trigPin = 9;
const int echoPin = 10;
const int ldrPin = A0;
const int redPin = 3;
const int greenPin = 5;
const int bluePin = 6;
const int buttonPin = 2;

// --- SETTINGS ---
const int clickThreshold = 300;

// --- OBJECTS ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 
LedControl lc = LedControl(11, 12, 13, 1);

// --- VARIABLES ---
float distance = 0;
float smoothDistance = 0;
float prevDistance = 0;

unsigned long lastScroll = 0;
unsigned long lastClick = 0;
unsigned long lastDisplay = 0;

// Toggle
bool isEngaged = false;
bool lastButtonState = HIGH;

const float alpha = 0.6; // smoother

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  lcd.init();
  lcd.backlight();

  lc.shutdown(0, false);
  lc.setIntensity(0, 5);
  lc.clearDisplay(0);

  lcd.setCursor(0, 0);
  lcd.print("STARK OS v8.0");
  lcd.setCursor(0, 1);
  lcd.print("STABLE MODE");
  setRGB(255,255,255);
  delay(1000);
  setRGB(0,0,0);
  lcd.clear();

  delay(5000);
  Mouse.begin();
}

void loop() {
  unsigned long now = millis();

  readDistance();
  int light = analogRead(ldrPin);

  // --- BUTTON TOGGLE ---
  bool currentButtonState = digitalRead(buttonPin);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    isEngaged = !isEngaged;
    delay(200);
  }
  lastButtonState = currentButtonState;

  // --- LCD ---
  lcd.setCursor(0, 0);
  lcd.print("D:");
  lcd.print((int)smoothDistance);
  lcd.print("cm   ");

  lcd.setCursor(8, 0);
  lcd.print("L:");
  lcd.print(light);
  lcd.print("   ");

  if (isEngaged) {
    lcd.setCursor(0, 1);
    lcd.print("MODE: ENGAGED   ");

    // RGB
    if (smoothDistance < 12) setRGB(255, 0, 0);
    else if (smoothDistance < 30) setRGB(0, 255, 0);
    else setRGB(0, 0, 255);

    // CLICK
    if (light < clickThreshold && (now - lastClick > 400)) {
      Mouse.click();
      lastClick = now;
    }

    // --- STABLE SCROLL (ZONE BASED) ---
    
    // DOWN
    if (smoothDistance > 6 && smoothDistance < 16) {
      showArrowDown();

      if (now - lastScroll > 80) {
        Mouse.move(0, 0, -1);
        lastScroll = now;
      }

      lastDisplay = now;
    }

    // UP
    else if (smoothDistance > 24 && smoothDistance < 36) {
      showArrowUp();

      if (now - lastScroll > 80) {
        Mouse.move(0, 0, 1);
        lastScroll = now;
      }

      lastDisplay = now;
    }

    else if (now - lastDisplay > 200) {
      lc.clearDisplay(0);
    }

  } else {
    lcd.setCursor(0, 1);
    lcd.print("MODE: LOCKED    ");
    setRGB(0,0,0);
    lc.clearDisplay(0);
  }

  prevDistance = smoothDistance;
}

// --- FUNCTIONS ---

void readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  smoothDistance = alpha * distance + (1 - alpha) * prevDistance;
}

void setRGB(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void showArrowUp() {
  byte u[8] = {0x18,0x3C,0x7E,0xFF,0x18,0x18,0x18,0x18};
  for(int i=0;i<8;i++) lc.setRow(0,i,u[i]);
}

void showArrowDown() {
  byte d[8] = {0x18,0x18,0x18,0x18,0xFF,0x7E,0x3C,0x18};
  for(int i=0;i<8;i++) lc.setRow(0,i,d[i]);
}