//version 0.1

#define WIFI_SSID "xxxxxxxxxxxx"
#define WIFI_PASS "xxxxxxxxxxxx"
#define BOT_TOKEN "xxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define CHAT_ID "xxxxxxxxx"


#include <FastBot.h>
FastBot bot(BOT_TOKEN);


bool butt_flag = 0;
bool butt;
bool led_flag = 0;
unsigned long last_press;

void setup() {

  pinMode(D2, INPUT);
  pinMode(D4, OUTPUT);



  connectWiFi();

  // можно сменить токен
  //bot.setToken(BOT_TOKEN);

  // можно сменить размер буфера на (приём, отправку), по умолч. 512, 512
  //bot.setBufferSizes(1024, 512);

  // установить ID чата, чтобы принимать сообщения только из него
  // узнать ID можно из ручного запроса в браузере
  bot.setChatID(CHAT_ID); // передай "" (пустую строку) чтобы отключить проверку

  // можно указать несколько ID через запятую
  //bot.setChatID("123456,7891011,12131415");

  // подключаем функцию-обработчик
  bot.attach(newMsg);

  // отправить сообщение в указанный в setChatID
  bot.sendMessage("Hello, World!");
}

// обработчик сообщений
void newMsg(FB_msg& msg) {
  // выводим имя юзера и текст сообщения
  //Serial.print(msg.username);
  //Serial.print(", ");
  //Serial.println(msg.text);

  // выводим всю информацию о сообщении
  Serial.println(msg.toString());


  if (msg.text == "/status" && led_flag == 1) {
    bot.sendMessage("нема світла дядя", msg.chatID);
  }


  if (msg.text == "/status" && led_flag == 0) {
    bot.sendMessage("світло на базі", msg.chatID);
    }

}


void loop() {

butt = digitalRead(D2); // считать текущее положение кнопки

  if (butt == 1 && butt_flag == 0 && millis() - last_press > 100) {
    butt_flag = 1;
    Serial.println("Button pressed");
    bot.sendMessage("Button pressed!");
    led_flag = !led_flag;
    digitalWrite(D4, led_flag);
    last_press = millis();
  }
  if (butt == 0 && butt_flag == 1) {
    butt_flag = 0;
    Serial.println("Button released");
    bot.sendMessage("Button released!");
  }



 bot.tick();   // тикаем в луп
}

void connectWiFi() {
  delay(2000);
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 15000) ESP.restart();
  }
  Serial.println("Connected");
}