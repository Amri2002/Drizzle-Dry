#define IR_PIN 8
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>

#define ONE_WIRE_BUS 2
#define RELAY_PIN_FAN 3
#define RELAY_PIN_HEAT1 6
#define RELAY_PIN_HEAT2 7
#define TRIG_PIN 13
#define ECHO_PIN 12

SoftwareSerial sim(10, 11);
LiquidCrystal_I2C lcd(0x27, 16, 2);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const char phoneNumber[] PROGMEM = "+94760301597";
int previousIrState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(IR_PIN, INPUT);
  pinMode(RELAY_PIN_FAN, OUTPUT);
  pinMode(RELAY_PIN_HEAT1, OUTPUT);
  pinMode(RELAY_PIN_HEAT2, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();
  Wire.setClock(10000);
  sim.begin(9600);
  delay(1000);
}

void loop() {
  displayMessage(F("Insert the"), F("Umbrella"));

  int irState = digitalRead(IR_PIN);

  if (irState == HIGH && previousIrState == LOW) {
    turnOffAllRelays();
  } else if (irState == LOW) {
    handleIrSensorLow(irState);
  }

  previousIrState = irState;
  delay(100);
}

void turnOffAllRelays() {
  digitalWrite(RELAY_PIN_FAN, LOW);
  digitalWrite(RELAY_PIN_HEAT1, LOW);
  digitalWrite(RELAY_PIN_HEAT2, LOW);
  Serial.println(F("All relays turned OFF"));
}

void handleIrSensorLow(int irState) {
  if (previousIrState == LOW) {
    turnOffAllRelays();
    delay(1000);
  } else {
    digitalWrite(RELAY_PIN_FAN, HIGH);
    Serial.println(F("Relay (fan) turned ON"));
    Serial.println(F("Detected"));
    handleHeatingCycle();
    displayMessage(F("Take Out the"), F("Umbrella"));
    // Turn off all relays after the countdown
    turnOffAllRelays();
    Serial.println(F("All relays turned OFF after countdown"));
    delay(5000);
    lcd.clear();
    measureDistanceAndSendSMS();
    turnOffAllRelays(); // Ensure all relays are turned off after the countdown and message display
  }
}

void handleHeatingCycle() {
  
  for (int time = 30; time >= 1; time--) {
    sensors.requestTemperatures();
    float temperatureC = sensors.getTempCByIndex(0);

    Serial.print(F("Temperature: "));
    Serial.println(temperatureC);

    if (temperatureC < 50) {
      digitalWrite(RELAY_PIN_HEAT1, HIGH);
      digitalWrite(RELAY_PIN_HEAT2, HIGH);
      Serial.println(F("Heating relays turned ON"));
      delay(250);
    } else {
      digitalWrite(RELAY_PIN_HEAT1, LOW);
      digitalWrite(RELAY_PIN_HEAT2, LOW);
      Serial.println(F("Heating relays turned OFF"));
      delay(250);
    }
-

..00001010011
    lcd.clear();
    lcd.setCursor(1, 1);
    lcd.print(time);
    delay(500);
    
  }


}

void measureDistanceAndSendSMS() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long cm = (duration / 2) / 29.1;

  Serial.print(F("Distance: "));
  Serial.print(cm);
  Serial.println(F(" cm"));

  if (cm < 3) {
    sendSMS(F("The Tray is Full"));
  }
}

void sendSMS(const __FlashStringHelper* message) {
  sim.println(F("AT+CMGF=1"));
  delay(200);
  sim.print(F("AT+CMGS=\""));
  sim.print(reinterpret_cast<const __FlashStringHelper*>(phoneNumber));
  sim.println(F("\""));
  delay(200);
  sim.println(message);
  delay(100);
  sim.println((char)26);
  delay(200);
  readSerial();
}

String readSerial() {
  int timeout = 0;
  while (!sim.available() && timeout < 12000) {
    delay(13);
    timeout++;
  }
  if (sim.available()) {
    return sim.readString();
  }
  return "";
}

void displayMessage(const __FlashStringHelper* line1, const __FlashStringHelper* line2) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(line1);
  lcd.setCursor(1, 1);
  lcd.print(line2);
}
