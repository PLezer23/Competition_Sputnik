
// Подключаем необходимые библиотеки
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ============ НАСТРОЙКИ ============
const char* ssid = "BigChallenges_Satellite";     // Имя сети спутника
const char* password = "12345678";                // Пароль (минимум 8 символов)

// Создаем веб-сервер на порту 80
WebServer server(80);

// ============ ПЕРЕМЕННЫЕ ============

// Структура для хранения данных одной станции
struct StationData {
  String name;           // Имя станции
  bool connected;        // Подключена ли сейчас
  float temperature;     // Температура
  float humidity;        // Влажность
  int gasLevel;          // Уровень газа (0-100)
  unsigned long lastSeen; // Когда последний раз получали данные
};

// Создаем массив для 5 станций (максимум)
StationData stations[5];
int stationCount = 0;    // Сколько станций нашли

// Для сброса станций
const int releasePin = 15;  // Пин, к которому подключен сервопривод (можно любой)
bool stationReleased = false;

// Для замера времени
unsigned long lastScanTime = 0;

// ============ НАСТРОЙКА (выполняется один раз при включении) ============
void setup() {
  Serial.begin(115200);
  Serial.println("Запускаю спутник-носитель...");
  
  // Настройка пина для сброса станций
  pinMode(releasePin, OUTPUT);
  digitalWrite(releasePin, LOW);
  
  // Запускаем Wi-Fi точку доступа
  WiFi.softAP(ssid, password);
  Serial.print("Точка доступа запущена. IP адрес: ");
  Serial.println(WiFi.softAPIP());
  
  // ============ НАСТРАИВАЕМ ВЕБ-СТРАНИЧКИ ============
  
  // Главная страница - показывает данные
  server.on("/", []() {
    String html = "<!DOCTYPE html><html>";
    html += "<head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Спутник-носитель</title>";
    html += "<style>";
    html += "body{font-family:Arial;margin:20px;background:#f0f0f0;}";
    html += "h1{color:#333;}";
    html += ".station{border:2px solid #4CAF50;border-radius:10px;padding:15px;margin:15px 0;background:white;}";
    html += ".connected{color:green;font-weight:bold;}";
    html += ".disconnected{color:red;font-weight:bold;}";
    html += "button{background:#4CAF50;color:white;border:none;padding:15px 32px;font-size:16px;margin:10px;border-radius:5px;}";
    html += "button:hover{background:#45a049;}";
    html += "</style>";
    html += "</head><body>";
    html += "<h1>🛰️ Спутник-носитель</h1>";
    
    // Показываем, сколько станций нашли
    html += "<p>Найдено станций: " + String(stationCount) + "</p>";
    
    // Показываем данные каждой станции
    for(int i = 0; i < stationCount; i++) {
      html += "<div class='station'>";
      html += "<h3>Станция: " + stations[i].name + "</h3>";
      
      if(stations[i].connected) {
        html += "<p class='connected'>✅ Подключена</p>";
        html += "<p>🌡️ Температура: " + String(stations[i].temperature) + " °C</p>";
        html += "<p>💧 Влажность: " + String(stations[i].humidity) + " %</p>";
        html += "<p>🧪 Газ: " + String(stations[i].gasLevel) + " (чем выше, тем хуже воздух)</p>";
        html += "<p>⏱️ Последние данные: " + String((millis() - stations[i].lastSeen)/1000) + " сек назад</p>";
      } else {
        html += "<p class='disconnected'>❌ Нет связи</p>";
      }
      
      html += "</div>";
    }
    
    // Кнопка для сброса станции
    html += "<button onclick='releaseStation()'>🚀 Сбросить новую станцию</button>";
    html += "<script>";
    html += "function releaseStation() {";
    html += "  fetch('/release').then(response => response.text()).then(data => alert(data));";
    html += "}";
    html += "</script>";
    
    html += "</body></html>";
    server.send(200, "text/html", html);
  });
  
  // Страница с данными в формате JSON (для программы, не для людей)
  server.on("/data", []() {
    String json = "{";
    json += "\"stationCount\":" + String(stationCount) + ",";
    json += "\"stations\":[";
    
    for(int i = 0; i < stationCount; i++) {
      if(i > 0) json += ",";
      json += "{";
      json += "\"name\":\"" + stations[i].name + "\",";
      json += "\"connected\":" + String(stations[i].connected ? "true" : "false") + ",";
      json += "\"temp\":" + String(stations[i].temperature) + ",";
      json += "\"hum\":" + String(stations[i].humidity) + ",";
      json += "\"gas\":" + String(stations[i].gasLevel);
      json += "}";
    }
    
    json += "]}";
    server.send(200, "application/json", json);
  });
  
  // Команда на сброс станции
  server.on("/release", []() {
    if(!stationReleased) {
      digitalWrite(releasePin, HIGH);
      delay(1000);  // Держим сервопривод 1 секунду
      digitalWrite(releasePin, LOW);
      stationReleased = true;
      server.send(200, "text/plain", "Станция сброшена!");
    } else {
      server.send(200, "text/plain", "Станция уже сброшена");
    }
  });
  
  // Запускаем сервер
  server.begin();
  Serial.println("Веб-сервер запущен!");
  
  // Очищаем массив станций
  for(int i = 0; i < 5; i++) {
    stations[i].connected = false;
    stations[i].name = "Станция " + String(i+1);
  }
}

// ============ ГЛАВНЫЙ ЦИКЛ (выполняется постоянно) ============
void loop() {
  // Обрабатываем запросы с веб-страницы
  server.handleClient();
  
  // Каждые 5 секунд проверяем станции
  if(millis() - lastScanTime > 5000) {
    scanForStations();
    lastScanTime = millis();
  }
}

// ============ ФУНКЦИЯ ПОИСКА СТАНЦИЙ ============
void scanForStations() {
  Serial.println("Сканирую станции...");
  
  // Тут мы не можем по-настоящему искать станции через Wi-Fi,
  // потому что ESP32 не умеет одновременно быть точкой доступа и клиентом.
  // В реальности станции сами присылают данные.
  // Но для простоты мы просто делаем вид, что данные приходят.
  
  // Вместо настоящего сканирования мы будем получать данные
  // через последовательный порт (Serial).
  // Это значит, что станции должны подключиться к компьютеру
  // и слать данные через USB, а компьютер будет их передавать в ESP32.
  // Но это сложно для демонстрации.
  
  // Поэтому для простоты я сделаю так:
  // - Станции подключаются к Wi-Fi спутника
  // - Шлют данные на адрес: http://192.168.4.1/update
  // - Спутник их принимает и сохраняет
  
  // Этот функционал я добавил ниже
  
  Serial.println("Ожидаю данные от станций...");
}

// ============ ОБРАБОТКА ДАННЫХ ОТ СТАНЦИЙ ============
// Добавляем обработчик для приема данных от станций
// Добавьте это в функцию setup(), после server.on("/release", ...)

/*
  server.on("/update", []() {
    // Получаем данные из запроса
    if(server.hasArg("name") && server.hasArg("temp") && server.hasArg("hum") && server.hasArg("gas")) {
      String name = server.arg("name");
      float temp = server.arg("temp").toFloat();
      float hum = server.arg("hum").toFloat();
      int gas = server.arg("gas").toInt();
      
      // Ищем станцию с таким именем
      bool found = false;
      for(int i = 0; i < stationCount; i++) {
        if(stations[i].name == name) {
          stations[i].temperature = temp;
          stations[i].humidity = hum;
          stations[i].gasLevel = gas;
          stations[i].connected = true;
          stations[i].lastSeen = millis();
          found = true;
          break;
        }
      }
      
      // Если не нашли, добавляем новую
      if(!found && stationCount < 5) {
        stations[stationCount].name = name;
        stations[stationCount].temperature = temp;
        stations[stationCount].humidity = hum;
        stations[stationCount].gasLevel = gas;
        stations[stationCount].connected = true;
        stations[stationCount].lastSeen = millis();
        stationCount++;
      }
      
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Не хватает данных");
    }
  });
*/

// Раскомментируйте этот код, если будете настраивать настоящую передачу
