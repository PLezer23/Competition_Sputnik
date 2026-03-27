
// Подключаем библиотеки
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// ============ ПОДКЛЮЧАЕМ ДАТЧИКИ ============
// Если у вас есть реальные датчики - раскомментируйте
// #include <DHT.h>
// #include <MQ135.h>

// ============ НАСТРОЙКИ ============
const char* ssid = "BigChallenges_Satellite";     // Имя сети спутника
const char* password = "12345678";                // Пароль
const char* serverName = "http://192.168.4.1/update"; // Адрес спутника

// Имя станции (можно поменять для каждой)
String stationName = "Station_1";  // Для второй станции: "Station_2", для третьей: "Station_3"

// ============ ПЕРЕМЕННЫЕ ============
WiFiClient wifiClient;
HTTPClient http;

// Для датчиков (если они есть)
// #define DHTPIN 2        // Пин, к которому подключен DHT22
// #define DHTTYPE DHT22   // Тип датчика
// DHT dht(DHTPIN, DHTTYPE);

// #define MQ135PIN A0      // Пин для MQ-135
// MQ135 mq135(MQ135PIN);

unsigned long lastMeasureTime = 0;

// ============ НАСТРОЙКА ============
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Запускаю метеостанцию " + stationName);
  
  // Запускаем датчики (если они есть)
  // dht.begin();
  
  // Подключаемся к Wi-Fi спутника
  WiFi.begin(ssid, password);
  Serial.print("Подключаюсь к спутнику");
  
  int attempts = 0;
  while(WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ Подключился к спутнику!");
    Serial.print("IP адрес: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("❌ Не удалось подключиться к спутнику");
  }
}

// ============ ГЛАВНЫЙ ЦИКЛ ============
void loop() {
  // Проверяем подключение к Wi-Fi
  if(WiFi.status() != WL_CONNECTED) {
    // Пробуем переподключиться
    WiFi.begin(ssid, password);
    delay(5000);
    return;
  }
  
  // Каждые 10 секунд отправляем данные
  if(millis() - lastMeasureTime > 10000) {
    sendData();
    lastMeasureTime = millis();
  }
  
  // Маленькая задержка, чтобы не нагружать процессор
  delay(100);
}

// ============ ФУНКЦИЯ ОТПРАВКИ ДАННЫХ ============
void sendData() {
  Serial.println("Собираю данные...");
  
  // ============ ЧИТАЕМ ДАТЧИКИ ============
  
  // Если у вас есть реальный DHT22 - используйте этот код
  /*
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  // Проверяем, не ошибка ли
  if(isnan(temperature) || isnan(humidity)) {
    Serial.println("Ошибка чтения DHT22");
    temperature = 25.0;  // Значение по умолчанию
    humidity = 50.0;
  }
  */
  
  // Если у вас нет датчиков - используйте случайные числа (для демонстрации)
  float temperature = 20.0 + random(0, 100) / 10.0;  // 20.0 - 30.0
  float humidity = 40.0 + random(0, 600) / 10.0;     // 40.0 - 100.0
  int gasLevel = random(0, 100);                      // 0 - 100
  
  // Если у вас есть реальный MQ-135 - используйте этот код
  /*
  float gasRaw = mq135.readCO2();  // Читаем CO2
  gasLevel = map(gasRaw, 400, 800, 0, 100);  // Преобразуем в проценты
  if(gasLevel < 0) gasLevel = 0;
  if(gasLevel > 100) gasLevel = 100;
  */
  
  // ============ ОТПРАВЛЯЕМ ДАННЫЕ ============
  
  Serial.println("Отправляю данные на спутник...");
  Serial.println("Температура: " + String(temperature) + " °C");
  Serial.println("Влажность: " + String(humidity) + " %");
  Serial.println("Газ: " + String(gasLevel));
  
  // Формируем адрес для отправки
  String url = String(serverName) + "?name=" + stationName + 
               "&temp=" + String(temperature) + 
               "&hum=" + String(humidity) + 
               "&gas=" + String(gasLevel);
  
  // Отправляем GET-запрос
  http.begin(wifiClient, url);
  int httpCode = http.GET();
  
  if(httpCode > 0) {
    String payload = http.getString();
    Serial.println("Ответ спутника: " + payload);
  } else {
    Serial.println("Ошибка отправки: " + String(http.errorToString(httpCode).c_str()));
  }
  
  http.end();
}
