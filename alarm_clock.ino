// include libraries
#include <LiquidCrystal.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <RTC.h>

// declare user variables
#define THRESHOLD     500
#define LED_COUNT     5
#define BRIGHTNESS    150 // NeoPixel brightness, 0 (min) to 255 (max) 
#define ALARM_HOUR    11
#define ALARM_MINUTE  34

// declare pins
const int forceSensorPin = A0;
const int ledPin = 8;
const int buzzerPin = 9;

// create objects
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
Adafruit_NeoPixel strip(LED_COUNT, ledPin, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
DS3231 RTC;

// declare variables
bool alarm, alarmState;
int lastSecond;
long lastAlarmTime;
char timeFormat[9], dateFormat[11];

void setup() {
  // initialize LCD
  lcd.begin(16, 2);

  // initialize NeoPixel
  strip.begin();  // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();   // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);

  // initialize clock and set to 12H mode
  RTC.begin();
  RTC.setHourMode(CLOCK_H12);

  // initialize pin
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH); // low trigger
}

void loop() {
  // update LCD every 1s
  if (RTC.getSeconds() != lastSecond) {
    updateLCD();

    if (RTC.getHours() == ALARM_HOUR && RTC.getMinutes() == ALARM_MINUTE && RTC.getSeconds() == 0) {
      alarm = true;
    }

    lastSecond = RTC.getSeconds();
  }

  if (alarm) {
    // toggle the alarm state every 100ms
    if (millis() - lastAlarmTime > 100) {
      alarmState = !alarmState;
      lastAlarmTime = millis();
    }

    // flash the LED on & off
    if (alarmState) {
      // show red colour
      strip.fill(strip.Color(255, 0, 0, strip.gamma8(255)));
    } else {
      // show blank
      strip.fill(strip.Color(0, 0, 0, strip.gamma8(255)));
    }
    strip.show();

    // toggle the buzzer on & off
    digitalWrite(buzzerPin, alarmState);

    if (checkSensor()) {
      alarm = false;
    }
  } else {
    // turn off LED
    strip.fill(strip.Color(0, 0, 0, strip.gamma8(255)));
    strip.show();
    // turn off buzzer
    digitalWrite(buzzerPin, HIGH);
  }
}

void updateLCD() {
  sprintf(timeFormat, "%02d:%02d:%02d", RTC.getHours(), RTC.getMinutes(), RTC.getSeconds());
  sprintf(dateFormat, "%02d/%02d/%04d", RTC.getDay(), RTC.getMonth(), RTC.getYear());

  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(timeFormat);
  if (RTC.getHourMode() == CLOCK_H12)
  {
    switch (RTC.getMeridiem())
    {
      case HOUR_AM :
        lcd.print(" AM");
        break;
      case HOUR_PM :
        lcd.print(" PM");
        break;
    }
  }

  lcd.setCursor(1, 1);
  switch (RTC.getWeek())
  {
    case 1:
      lcd.print("SUN");
      break;
    case 2:
      lcd.print("MON");
      break;
    case 3:
      lcd.print("TUE");
      break;
    case 4:
      lcd.print("WED");
      break;
    case 5:
      lcd.print("THU");
      break;
    case 6:
      lcd.print("FRI");
      break;
    case 7:
      lcd.print("SAT");
      break;
  }
  lcd.setCursor(5, 1);
  lcd.print(dateFormat);
}

// get an average reading from sensor for stability
bool checkSensor() {
  float average = 0;
  for (int i = 0; i < 25; i++) {
    average += analogRead(forceSensorPin);
  }
  average = average / 25.0;

  if (average > THRESHOLD) return true;
  return false;
}
