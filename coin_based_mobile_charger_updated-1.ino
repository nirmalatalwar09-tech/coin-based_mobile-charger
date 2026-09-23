#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int coinPin = 2;
const int relayPin = 8;

unsigned long chargeTime = 0;
unsigned long lastCoinTime = 0;

// 5 minutes = 300000 ms
const unsigned long coinValue = 300000;

const unsigned long lockTime = 1000;

unsigned long blockStart = 0;
bool beamBlocked = false;

bool timeJustEnded = false;
unsigned long timeOverStart = 0;

void setup() {
  pinMode(coinPin, INPUT);
  pinMode(relayPin, OUTPUT);

  digitalWrite(relayPin, HIGH); // relay OFF

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("Insert Coin");
}

void loop() {
  bool sensorState = digitalRead(coinPin) == LOW;

  // detect beam block
  if (sensorState && !beamBlocked) {
    beamBlocked = true;
    blockStart = millis();
  }

  // detect release
  if (!sensorState && beamBlocked) {
    unsigned long blockTime = millis() - blockStart;
    beamBlocked = false;

    if (blockTime > 50 && blockTime < 300) {
      if (millis() - lastCoinTime > lockTime) {
        chargeTime += coinValue;
        lastCoinTime = millis();
        timeJustEnded = false;
      }
    }
  }

  // anti-hold protection
  if (sensorState && (millis() - blockStart > 1000)) {
    beamBlocked = false;
  }

  // relay control (active LOW)
  if (chargeTime > 0) {
    digitalWrite(relayPin, LOW);
  } else {
    digitalWrite(relayPin, HIGH);
  }

  // countdown timer
  static unsigned long lastTimer = 0;

  if (millis() - lastTimer >= 1000 && chargeTime > 0) {
    chargeTime -= 1000;
    lastTimer = millis();

    if (chargeTime == 0) {
      timeJustEnded = true;
      timeOverStart = millis();
    }
  }

  // LCD display
  lcd.setCursor(0, 0);

  if (chargeTime > 0) {
    lcd.print("Charging...");
  } else if (timeJustEnded) {
    lcd.print("TIME OVER");
    if (millis() - timeOverStart > 3000) {
      timeJustEnded = false;
    }
  } else {
    lcd.print("Insert Coin");
  }

  // time display in minutes and seconds
  int seconds = chargeTime / 1000;
  int minutes = seconds / 60;
  seconds = seconds % 60;

  lcd.setCursor(0, 1);
  lcd.print("Time: ");

  if (minutes < 10) {
    lcd.print("0");
  }
  lcd.print(minutes);
  lcd.print(":");

  if (seconds < 10) {
    lcd.print("0");
  }
  lcd.print(seconds);
}
