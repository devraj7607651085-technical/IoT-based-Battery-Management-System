#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <HTTPClient.h>

const int redLed = 25;
const int greenLed = 26;
const int buzzer = 27;
const int potPin = 34;
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const int relayPin = 14;

String apiKey = "AAO7Z3K6ER7DZ8D9";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED Failed");
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(relayPin, OUTPUT);
digitalWrite(relayPin, HIGH); // Normal state = Load ON

  sensors.begin();
  sensors.setResolution(10);
  WiFi.begin(ssid, password);

Serial.print("Connecting to WiFi");

while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

Serial.println("");
Serial.println("WiFi Connected!");
}

void loop() {

  // Voltage Reading
  int adcValue = analogRead(potPin);
  float voltage = adcValue * (3.3 / 4095.0);

  // Battery Percentage
  int batteryPercent = (voltage / 3.3) * 100;
  batteryPercent = constrain(batteryPercent, 0, 100);

  // Temperature Reading
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  // Alert Logic
  if (batteryPercent < 20 || temperature > 45) {

  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);

  tone(buzzer, 1000);

  digitalWrite(relayPin, LOW);   // Load OFF
}
else {

  digitalWrite(redLed, LOW);
  digitalWrite(greenLed, HIGH);

  noTone(buzzer);

  digitalWrite(relayPin, HIGH);  // Load ON
}

  // Serial Monitor
  Serial.print("Voltage: ");
  Serial.print(voltage, 2);
  Serial.print("V  Battery: ");
  Serial.print(batteryPercent);
  Serial.print("%  Temp: ");
  Serial.print(temperature);
  Serial.println("C");

  // OLED Display
  display.clearDisplay();
  display.setTextSize(1);

  // Voltage
  display.setCursor(0, 5);
  display.print("Voltage:");
  display.print(voltage, 2);
  display.print("V");

  // Battery Icon
  display.drawRect(80, 15, 40, 20, SSD1306_WHITE);
  display.fillRect(120, 20, 4, 10, SSD1306_WHITE);

  int fillWidth = map(batteryPercent, 0, 100, 0, 38);
  display.fillRect(81, 16, fillWidth, 18, SSD1306_WHITE);

  // Battery Percentage
  display.setCursor(0, 30);
  display.print("Battery:");
  display.print(batteryPercent);
  display.print("%");

  // Temperature
  display.setCursor(0, 45);
  display.print("Temp:");
  display.print(temperature, 1);
  display.print("C");
  // Relay Status
display.setCursor(70, 45);
display.print("R:");

if (batteryPercent < 20 || temperature > 45)
  display.print("OFF");
else
  display.print("ON");

  // Low Battery Warning
  
  if (temperature > 45) {
  display.setCursor(0, 56);
  display.print("OVER TEMP!");
}
else if (batteryPercent < 20) {
  display.setCursor(0, 56);
  display.print("LOW BATTERY!");
}

  display.display();
  if (WiFi.status() == WL_CONNECTED) {

  HTTPClient http;

  String url =
    "http://api.thingspeak.com/update?api_key=" + apiKey +
    "&field1=" + String(voltage, 2) +
    "&field2=" + String(batteryPercent) +
    "&field3=" + String(temperature, 1);

  http.begin(url);

  int httpResponseCode = http.GET();

  Serial.print("ThingSpeak Response: ");
  Serial.println(httpResponseCode);

  http.end();
}

  delay(20000);
}