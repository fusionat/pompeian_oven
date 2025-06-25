#include <max6675.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

#define LED D8
// Настройки WiFi
const char* ssid = "*";
const char* password = "*";


// Инициализация веб-сервера
ESP8266WebServer server(80);

// Контакты MAX6675
const int thermoCLK = D5;
const int thermoCS = D6;
const int thermoDO = D7;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

IPAddress local_IP(192, 168, 49, 211);  // Ваш ESP
IPAddress gateway(192, 168, 49, 1);     // Обычно это роутер
IPAddress subnet(255, 255, 255, 0);    // Стандартная маска для домашней сети

// Хранение данных температуры
const int MAX_POINTS = 300; // 180 точек = 3 часа (1 мин интервал)
float temperatureHistory[MAX_POINTS];
int currentIndex = 0;
unsigned long lastUpdateTime = 0;
const long updateInterval = 3000; // 2 минута
float currentTemp = 0.0;

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);

  // Настраиваем статический IP
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
WiFi.setAutoConnect(true);  // Автоподключение при старте
WiFi.setAutoReconnect(true); // Автопереподключение при обрыве
  // Подключение к WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED, LOW);
    delay(500);
    Serial.print(".");
  }
  
  digitalWrite(LED, HIGH);

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Быстрое мигание 3 раза при подключении
  for(int i=0; i<50; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
  }
  // Инициализация истории температуры
  for (int i = 0; i < MAX_POINTS; i++) {
    temperatureHistory[i] = 0.0;
  }

currentTemp = thermocouple.readCelsius();

  // Настройка эндпоинтов веб-сервера
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/current", handleCurrent);
  server.begin();
}

void loop() {
  server.handleClient();
  
  unsigned long currentTime = millis();
  if (currentTime - lastUpdateTime >= updateInterval) {
    lastUpdateTime = currentTime;
    
    float temp = thermocouple.readCelsius();
    
    if (!isnan(temp)) {
      currentTemp = temp;
      
      // Сохраняем температуру в историю
      temperatureHistory[currentIndex] = temp;
      currentIndex = (currentIndex + 1) % MAX_POINTS;
      
      // Вывод в Serial
      Serial.print("Temperature: ");
      Serial.print(temp);
      Serial.println(" °C");
    }
  }
}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='60'>";
  html += "<title>Temperature Monitor</title>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  html += "<style>body{font-family:Arial,sans-serif;margin:20px;}</style>";
  html += "</head><body>";
  html += "<h1>Temperature Monitoring</h1>";
  html += "<div style='font-size: 24px; margin-bottom: 20px;'>Current Temperature: <span id='currentTemp'>" + String(currentTemp) + "</span></div>";
  html += "<div style='width:90%;height:400px;'><canvas id='tempChart'></canvas></div>";
  html += "<script>"
          "const ctx = document.getElementById('tempChart').getContext('2d');"
          "const tempChart = new Chart(ctx, {"
          "  type: 'line',"
          "  data: {"
          "    labels: Array(300).fill('').map((_,i)=>i+'m'),"
          "    datasets: [{"
          "      label: 'Temperature (°C)',"
          "      borderColor: 'rgb(75, 192, 192)',"
          "      data: [],"
          "      fill: false"
          "    }]"
          "  },"
          "  options: {"
          "    responsive: true,"
          "    scales: {"
          "      x: { title: { display: true, text: 'Time (minutes ago)' } },"
          "      y: { title: { display: true, text: 'Temperature (°C)' } }"
          "    }"
          "  }"
          "});"
          "function updateData() {"
          "  fetch('/data').then(r=>r.json()).then(data=>{"
          "    tempChart.data.datasets[0].data = data;"
          "    tempChart.update();"
          "  });"
          "  fetch('/current').then(r=>r.text()).then(temp=>{"
          "    document.getElementById('currentTemp').textContent = temp;"
          "  });"
          "}"
          "updateData();" 
          "setInterval(updateData, 60000);"
          "</script>";
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

void handleData() {
  DynamicJsonDocument doc(1024);
  JsonArray array = doc.to<JsonArray>();
  
  // Собираем данные в правильном порядке (старые -> новые)
  for (int i = 0; i < MAX_POINTS; i++) {
     if (temperatureHistory[i] == 0.0) {
      break;
     }
    array.add(temperatureHistory[i]);
  }
  
  String output;
  serializeJson(doc, output);
  server.send(200, "application/json", output);
}

void handleCurrent() {
  float ct = thermocouple.readCelsius();
  server.send(200, "text/plain", String(ct));
}