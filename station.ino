#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22
#define MQ_PIN A0

const char* ssid = "SPUTNIK_SATELLITE";
const char* password = "sputnik2026";
const char* serverUrl = "http://192.168.4.1/data";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient client;

String stationId = "ST" + String(ESP.getChipId());
float temp, hum;
int air;

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(MQ_PIN, INPUT);
  
  connectToNetwork();
}

void loop() {
  readSensors();
  sendData();
  delay(5000);
}

void connectToNetwork() {
  WiFi.begin(ssid, password);
  
  Serial.print("Подключение к ");
  Serial.println(ssid);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nПодключено!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nОшибка подключения");
  }
}

void readSensors() {
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  air = analogRead(MQ_PIN);
  
  if (!isnan(temp) && !isnan(hum)) {
    Serial.print("T: ");
    Serial.print(temp);
    Serial.print("C H: ");
    Serial.print(hum);
    Serial.print("% Air: ");
    Serial.println(air);
  } else {
    temp = 0;
    hum = 0;
  }
}

void sendData() {
  if (WiFi.status() == WL_CONNECTED) {
    String data = "{\"id\":\"" + stationId + "\",";
    data += "\"temp\":" + String(temp) + ",";
    data += "\"hum\":" + String(hum) + ",";
    data += "\"air\":" + String(air) + "}";
    
    HTTPClient http;
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");
    
    int code = http.POST(data);
    if (code == 200) {
      Serial.println("Данные отправлены");
    } else {
      Serial.print("Ошибка: ");
      Serial.println(code);
    }
    
    http.end();
  } else {
    Serial.println("Нет подключения");
    connectToNetwork();
  }
}
