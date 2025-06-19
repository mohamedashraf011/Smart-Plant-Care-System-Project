#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>
#include <DHT.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* server = "http://api.thingspeak.com/update";
const char* apiKey = "22IX8XCNQPFB1WKQ";

#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define SOIL_SENSOR_PIN 34

#define SD_CS 5

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Welcome Smart Plant Care System");
  delay(2000);
  lcd.clear();
  lcd.print("Initializing...");
  delay(2000);
  lcd.clear();
  lcd.print("Connecting to WiFi");

  WiFi.begin(ssid, password, 6);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    lcd.setCursor(0, 1);
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected!");
  Serial.println(" Connected!");

  dht.begin();

  if (!SD.begin(SD_CS)) {
    Serial.println("SD Card initialization failed!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SD Init Failed!");
    while (true);
  }
  Serial.println("SD Card initialized.");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
}

void logData(float temperature, float humidity, float soilMoisture) {
  File dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.print("Temperature: ");
    dataFile.print(temperature);
    dataFile.print(" C, Humidity: ");
    dataFile.print(humidity);
    dataFile.print(" %, Soil Moisture: ");
    dataFile.println(soilMoisture);
    dataFile.close();
    Serial.println("Data logged to SD Card.");
  } else {
    Serial.println("Failed to open data.csv");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SD Write Error!");
  }
}

void sendDataToThingSpeak(float temperature, float humidity, float soilMoisture) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey + "&field1=" + String(temperature) +
                 "&field2=" + String(humidity) + "&field3=" + String(soilMoisture);

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.println("Data sent to ThingSpeak!");
    } else {
      Serial.println("Error sending data: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void displayData(float temperature, float humidity, float soilMoisture) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C");

  lcd.setCursor(0, 1);
  lcd.print("Hum: ");
  lcd.print(humidity);
  lcd.print(" %");

  lcd.setCursor(0, 2);
  lcd.print("Soil: ");
  lcd.print(soilMoisture);
  lcd.print(" %");

  lcd.setCursor(0, 3);
  lcd.print("System OK");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  int soilMoistureRaw = analogRead(SOIL_SENSOR_PIN);
  float soilMoisture = map(soilMoistureRaw, 0, 4095, 0, 100);

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");
    delay(2000);
    return;
  }

  displayData(temperature, humidity, soilMoisture);
  logData(temperature, humidity, soilMoisture);
  sendDataToThingSpeak(temperature, humidity, soilMoisture);

  delay(15000);
}
