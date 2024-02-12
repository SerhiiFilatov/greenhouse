#include <Wire.h> 
#include <PCF8574_HD44780_I2C.h>
#include "DHT.h"
#include <RTClib.h>

#define DHT_PIN 3
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

RTC_DS3231 rtc;
PCF8574_HD44780_I2C lcd(0x27, 16, 2);

const int button_lcd_mode = 9;
const int button_cooler_mode = 7;
const int button_hydro_mode = 7;
const int relayPin = 11;
const int relay_fog_pin = 8;
bool lcd_status_temperature = true;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // Устанавливаем время задержки дебаунса в миллисекундах

void setup() {
  Serial.begin(9600);
  pinMode(button_lcd_mode, INPUT_PULLUP);
  pinMode(button_cooler_mode, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  pinMode(relay_fog_pin, OUTPUT);

  digitalWrite(relayPin, HIGH);
  digitalWrite(relay_fog_pin, HIGH);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.init();
  lcd.backlight();
  lcd.clear();
  dht.begin();
}

void loop() {
  DateTime now = rtc.now();
  float humidity = dht.readHumidity();
  float celsius = dht.readTemperature();

  int buttonState = digitalRead(button_lcd_mode);
  int button_cooler_state = digitalRead(button_cooler_mode);

  // Дебаунс кнопки
  if (buttonState == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    // При каждом нажатии кнопки меняем режим
    lcd_status_temperature = !lcd_status_temperature;
    lastDebounceTime = millis(); // Сохраняем время последнего изменения состояния
  }

  if (button_cooler_state == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    digitalWrite(relayPin, LOW);
    lastDebounceTime = millis(); 
  }else {
    if (humidity >= 60.00) {
      digitalWrite(relayPin, LOW);
    }else{
      digitalWrite(relayPin, HIGH);
    }
  }

  if (button_cooler_state == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    digitalWrite(relayPin, LOW);
    lastDebounceTime = millis(); 
  }else {
    if (humidity >= 60.00) {
      digitalWrite(relayPin, LOW);
    }else{
      digitalWrite(relayPin, HIGH);
    }
  }


  if (humidity < 40.00) {
    digitalWrite(relay_fog_pin, LOW);
  }else {
    digitalWrite(relay_fog_pin, HIGH);
  }

  if (lcd_status_temperature) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("h:");
    lcd.print(humidity);
    lcd.print(" % ");

    lcd.setCursor(0,1);
    lcd.print("t:");
    lcd.print(celsius);
    lcd.print(" c ");
  } else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(now.day(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.year() % 100, DEC);

    lcd.setCursor(0,1);
    lcd.print(now.hour() < 10 ? "0" : "");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute() < 10 ? "0" : "");
    lcd.print(now.minute(), DEC);
  }

  delay(1000);  // Добавим небольшую задержку между обновлениями
}