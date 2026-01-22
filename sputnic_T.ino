#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

const char* ssid = "SPUTNIK_SATELLITE";
const char* password = "sputnik2026";

struct StationData {
  String id;
  float temp;
  float hum;
  int air;
  unsigned long time;
};

StationData stations[10];
int stationCount = 0;
bool networkCreated = false;

void setup() {
  Serial.begin(115200);
  
  createNetwork();
  
  server.on("/data", HTTP_POST, handleData);
  server.on("/status", HTTP_GET, handleStatus);
  server.begin();
  
  Serial.println("Сервер запущен");
}

void loop() {
  server.handleClient();
  
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 10000) {
    checkStations();
    lastCheck = millis();
  }
}

void createNetwork() {
  if (!networkCreated) {
    Serial.println("Создание сети...");
    
    if (WiFi.softAP(ssid, password)) {
      IPAddress ip = WiFi.softAPIP();
      Serial.print("Сеть создана: ");
      Serial.println(ssid);
      Serial.print("IP адрес: ");
      Serial.println(ip);
      Serial.print("MAC: ");
      Serial.println(WiFi.softAPmacAddress());
      
      networkCreated = true;
    } else {
      Serial.println("Ошибка создания сети!");
    }
  }
}

void handleData() {
  if (server.hasArg("plain")) {
    String data = server.arg("plain");
    
    String stationId = getValue(data, "id");
    float temp = getValue(data, "temp").toFloat();
    float hum = getValue(data, "hum").toFloat();
    int air = getValue(data, "air").toInt();
    
    bool found = false;
    for (int i = 0; i < stationCount; i++) {
      if (stations[i].id == stationId) {
        stations[i].temp = temp;
        stations[i].hum = hum;
        stations[i].air = air;
        stations[i].time = millis();
        found = true;
        break;
      }
    }
    
    if (!found && stationCount < 10) {
      stations[stationCount].id = stationId;
      stations[stationCount].temp = temp;
      stations[stationCount].hum = hum;
      stations[stationCount].air = air;
      stations[stationCount].time = millis();
      stationCount++;
      
      Serial.print("Новая станция подключена: ");
      Serial.println(stationId);
    }
    
    Serial.print("Данные от ");
    Serial.print(stationId);
    Serial.print(": T=");
    Serial.print(temp);
    Serial.print("C H=");
    Serial.print(hum);
    Serial.print("% A=");
    Serial.println(air);
    
    server.send(200, "text/plain", "OK");
  }
}

void handleStatus() {
  String response = "{\"stations\":[";
  for (int i = 0; i < stationCount; i++) {
    if (i > 0) response += ",";
    response += "{\"id\":\"" + stations[i].id + "\",";
    response += "\"temp\":" + String(stations[i].temp) + ",";
    response += "\"hum\":" + String(stations[i].hum) + ",";
    response += "\"air\":" + String(stations[i].air) + ",";
    response += "\"age\":" + String((millis() - stations[i].time) / 1000) + "}";
  }
  response += "],\"network\":\"" + String(ssid) + "\"}";
  
  server.send(200, "application/json", response);
}

void checkStations() {
  unsigned long now = millis();
  for (int i = 0; i < stationCount; i++) {
    if (now - stations[i].time > 60000) {
      Serial.print("Станция ");
      Serial.print(stations[i].id);
      Serial.println(" неактивна");
    }
  }
}

String getValue(String data, String key) {
  String search = "\"" + key + "\":";
  int start = data.indexOf(search);
  if (start == -1) return "";
  
  start += search.length();
  
  char nextChar = data.charAt(start);
  if (nextChar == '"') {
    start++;
    int end = data.indexOf("\"", start);
    return data.substring(start, end);
  } else {
    int end = data.indexOf(",", start);
    if (end == -1) end = data.indexOf("}", start);
    return data.substring(start, end);
  }
}
