#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiClientSecure.h>
//Библиотека файловой системы модуля
#include "FS.h"
// WiFiFlientSecure для поддержки SSL/TLS
WiFiClientSecure client;

//Данные для подключения к беспроводной сети
const char *ssid = "HeX";
const char *password = "1234567890325";
const String filename = "/data";

int nextConnectTime = 0;
//Переменная, задающая время задержки до следующего подключения к серверу
//60 минут
//int reconnectioTime = 3600000;
int reconnectioTime = 60000;
ESP8266WebServer server(80);

//Светодиод-индикатор работы веб-сервера
const int led = 13;

void setup (void) {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  //Присоединяемся к точке доступа
  if (connect_to_wifi() != WL_CONNECTED) {
    Serial.println("ERROR: Not connected to WiFi Access Point. Stopping executing");
    stop_exec();
  }

  //Настраиваем веб-сервер
  Serial.println("Starting the web server");
  set_server();
  Serial.println("HTTP server started");
  //Монтируем файловую систему
  Serial.println("Mounting FS");
  if (SPIFFS.begin())
    Serial.println("FileSystem successfully mounted");
  else {
    Serial.println("ERROR: Cannot mount FileSystem. Stopping executing");
    stop_exec();
  }
  //Инициализируем соединение с Uno
  Serial1.begin(9600);
  Serial.println("Init finished. Start working.");
}

void loop() {
  //обслуживаем клиента, если он есть
  server.handleClient();
  //Проверяем, пришло ли время для нового соединения
  if (itIsTimeToConnect()) {
    Serial.println("It is time to check new messages");
    //Выполняем соединения
    doConnection();
    Serial.println("All the emails checked");
    Serial.printf("Waiting %d minutes before next connection", reconnectioTime / 60000);
    Serial.println();
  }

}

void stop_exec() {
  while (1)
    yield();
}

/**
   returns:
   0 : WL_IDLE_STATUS - when Wi-Fi is in process of changing between statuses
   1 : WL_NO_SSID_AVAIL - in case configured SSID cannot be reached
   3 : WL_CONNECTED - after successful connection is established
   4 : WL_CONNECT_FAILED - if password is incorrect
   6 : WL_DISCONNECTED - if module is not configured in station mode
*/
int connect_to_wifi() {
  Serial.println();
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  Serial.println();
  //Ждем соенинение до минуты
  int time_to_wait = millis() + 60000;
  while (millis() < time_to_wait) {
    //Соединение установлено?
    if (WiFi.status() == WL_CONNECTED)
      break;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Successfully connected!!");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("WiFi configuration info:");
    WiFi.printDiag(Serial);
    Serial.println();
    return WiFi.status();
  }
  else if (WiFi.status() == WL_IDLE_STATUS || WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_DISCONNECTED) {
    switch (WiFi.status()) {
      case WL_IDLE_STATUS: {
          Serial.println("Cannot connect to WiFi: current conncetion status is WL_IDLE_STATUS");
        } break;
      case WL_NO_SSID_AVAIL: {
          Serial.println("Cannot connect to WiFi: current conncetion status is WL_NO_SSID_AVAIL");
        } break;
      case WL_CONNECT_FAILED: {
          Serial.println("Cannot connect to WiFi: current conncetion status is WL_CONNECT_FAILED");
        } break;
      case WL_DISCONNECTED: {
          Serial.println("Cannot connect to WiFi: current conncetion status is WL_DISCONNECTED");
        } break;
    }
    return WiFi.status();
  }

}
void set_server() {
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
}

void handleRoot() {
  //Включаем светодиод на время обработки запроса
  digitalWrite (led, 1);
  String temp = "<html>\
  <head>\
    <title>ESP8266</title>\
  </head>\
  <body>\
    <h2>Enter some data to connect to mail server</h2>\
      <form method=\"post\">\
        <input type=\"hidden\" name=\"type\" value=\"mail\"></input><br />\
        <input type=\"text\" name=\"url\" required>Your mail provider server domain name</input><br />\
        <input type=\"text\" name=\"port\" required>Your mail provider server port</input><br />\
        <input type=\"text\" name=\"login\" required>Your mail login</input><br />\
        <input type=\"password\" name=\"pass\" required>Your mail password</input><br />\
        <input type=\"submit\">Send</input>\
      </form>\
  </body>\
</html>";
  server.send(200, "text/html", temp);
  //Проверка на получение параметров в запросе
  if (server.args() > 0) {
    Serial.println("Received some data from server");
    Serial.println("Procesing the received data:");
    if (server.arg("type") == "mail") {
      if (server.arg("url"))
        Serial.println("Mail IP " + server.arg("url"));
      if (server.arg("port"))
        Serial.println("Mail port " + server.arg("port"));
      if (server.arg("login"))
        Serial.println("Mail server login: " + server.arg("login"));
      if (server.arg("pass"))
        Serial.println("Mail server pass: " + server.arg("pass"));
      //Добавляем запись в файл
      Serial.println("Appending data to file");
      writeFileRecord(server.arg("type"), server.arg("url"), server.arg("port"), server.arg("login"), server.arg("pass"));
    }
    else {
      Serial.println("Received data is invalid");
    }
  }
  //Выключаем светодиод
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

/**
   Эта функция добавляет новую запись в файл с записями об серверах и параметрах доступа к ним
*/
bool writeFileRecord(String type, String url, String port, String login, String pass) {
  //Открываем файл "/data"
  File f = SPIFFS.open(filename, "a");
  if (!f) {
    Serial.println("Opening data file failed");
    return false;
  }
  type.trim();
  url.trim();
  port.trim();
  login.trim();
  pass.trim();
  f.println("Type:" + type);
  f.println("Url:" + url);
  f.println("Port:" + port);
  f.println("Login:" + login);
  f.println("Pass:" + pass);
  //Закрываем файл. Никакие операции с ним уже не могут быть произведены
  f.close();
  return true;
}

/**
   Эта функция читает из файла данных 5 параметров:
   - Тип подключения
   - Адрес сервера для подключения
   - Порт для подключения
   - Логин пользователя
   - Пароль пользователя
*/
bool readFileRecord(String strings[], File f) {
  String s = f.readStringUntil('\n');
  if (s.startsWith("Type:")) {
    int pos = s.indexOf(":");
    strings[0] = s.substring(pos + 1, s.length() - 1);
    if (s.substring(pos + 1, s.length() - 1).equals("mail")) {
      Serial.println("YESSSSSSSSSSSSSSSs");
      for (int i = 1; i < 5; i++) {
        s = f.readStringUntil('\n');
        pos = s.indexOf(":");
        strings[i] = s.substring(pos + 1, s.length() - 1);
      }
    }
  }
  else {
    return false;
  }
  return true;
}

/**
   Эта функция возвращает true, если время задержки между отправками запросов на сервер закончилось
*/
bool itIsTimeToConnect() {
  //Счетчик millis() переполняется приблизительно через 50 дней
  if (millis() >= nextConnectTime) {
    nextConnectTime = millis() + reconnectioTime;
    return true;
  }
  return false;
}

void doConnection() {
  String fileData[5];
  //Открываем файл
  File f = SPIFFS.open(filename, "r");
  if (!f) {
    Serial.println("Opening data file failed");
    return;
  }
  yield();

  //Пока есть записи в файле, получаем их
  while (readFileRecord(fileData, f)) {
    yield();
    //Проверяем тип сервиса
    //if(fileData[0].c_str()=="mail"){
    Serial.println("Checking IMAP");
    connectIMAP(fileData);
    //}
    delay(5000);
    Serial.println("Checking another email address");
  }
  f.close();
}

void connectIMAP(String conn_info[]) {
  const char* d1 = conn_info[1].c_str();
  int d2 = atoi(conn_info[2].c_str());
  Serial.print("[ MAIL IMAP ] Connecting to ");
  Serial.print(d1);
  Serial.print(":");
  Serial.println(d2);
  if (!client.connect(d1, d2)) {
    Serial.println("[ MAIL IMAP ] ERROR: connecting failed");
    return;
  }
  Serial.println("[ MAIL IMAP ] Connecting succeeded");
  //Задарживаемся на некоторое время, чтобы сервер обработал запрос
  delay(3000);
  //Выводим ответ сервера
  while (client.available()) {
    Serial.println(">> " + client.readStringUntil('\n'));
    yield();
  }
  //Выполняем авторизацию
  client.print(String("A login " + conn_info[3] + " " + conn_info[4]) + "\r\n");
  Serial.println("<< " + String("A login " + conn_info[3] + " ****************"));
  delay(3000);
  //Выводим ответ сервера
  while (client.available()) {
    Serial.println(">> " + client.readStringUntil('\n'));
    yield();
  }
  //Запрашиваем содержимое почтового ящика
  //STATUS
  //client.print(String("B STATUS INBOX (RECENT UNSEEN MESSAGES)") + "\r\n");
  //Serial.println("<< " + String("B STATUS INBOX (RECENT UNSEEN MESSAGES)") + "\r\n");
  client.print(String("B STATUS INBOX (UNSEEN)") + "\r\n");
  Serial.println("<< " + String("B STATUS INBOX (UNSEEN)") + "\r\n");
  delay(3000);
  String ans;
  String numUnseen;
  //Выводим ответ сервера
  while (client.available()) {
    ans = client.readStringUntil('\n');
    if(ans.startsWith("*")){
      numUnseen = ans.substring(ans.indexOf("UNSEEN"), ans.length() - 2);
    }
    Serial.println(">> " + ans);
    yield();
  }
  //Завершаем соединение
  client.print(String("C logout") + "\r\n");
  Serial.println("<<" + String("C logout") + "\r\n");
  delay(3000);
  //Выводим ответ сервера
  while (client.available()) {
    Serial.println(">> " + client.readStringUntil('\n'));
    yield();
  }
  Serial.println("[ MAIL IMAP ] Connecting closed");
  //Передаем данные на ардуино
  Serial1.println(conn_info[1] + " " +  numUnseen + " ");
}
