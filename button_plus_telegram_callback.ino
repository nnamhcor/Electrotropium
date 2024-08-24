#define ver "version 0.4"

#define WIFI_SSID "xxxxxxxxxxxx"
#define WIFI_PASS "xxxxxxxxxxxx"
#define BOT_TOKEN "xxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define CHAT_ID "xxxxxxxxx"


#include <FastBot.h>

FastBot bot(BOT_TOKEN);


bool butt_flag = 1;
//bool last_butt_flag = 0;
bool butt;
bool led_flag = 0;
unsigned long last_press;

uint32_t startUnix;     // save time


void setup() {

  pinMode(4, INPUT);
  pinMode(2, OUTPUT);
  pinMode(16, OUTPUT);
  digitalWrite(16, TRUE);
  connectWiFi();

  // можно сменить размер буфера на (приём, отправку), по умолч. 512, 512
  //bot.setBufferSizes(1024, 512);

  bot.setChatID(CHAT_ID); // передай "" (пустую строку) чтобы отключить проверку

  //bot.setChatID("123456,7891011,12131415");

  // подключаем функцию-обработчик
  bot.attach(newMsg);

  // отправить сообщение в указанный в setChatID
  bot.sendMessage("*Hello, world*");
  bot.sendMessage(ver);

  startUnix = bot.getUnix(); 
}

// обработчик сообщений
void newMsg(FB_msg& msg) {
  // выводим имя юзера и текст сообщения
  //Serial.print(msg.username);
  //Serial.print(", ");
  //Serial.println(msg.text);
  
  // выводим всю информацию о сообщении
  //Serial.println(msg.toString());

  if (msg.unix < startUnix) return; // ignor massenge

  if (msg.text == "/status" && led_flag == 1) {
    bot.sendMessage("нема світла дядя", msg.chatID);
    
  }


  if (msg.text == "/status" && led_flag == 0) {
    bot.sendMessage("світло на базі", msg.chatID);
    }
  
}
  

void loop() {

butt = digitalRead(4); // считать текущее положение кнопки
  
  if (butt == 1 && butt_flag == 0 && millis() - last_press > 100) {
    butt_flag = 1;
    //Serial.println("Button pressed");
    bot.sendMessage("Button pressed!");
    led_flag = !led_flag;
    digitalWrite(2, led_flag);
    last_press = millis();
  }
  if (butt == 0 && butt_flag == 1) {
    butt_flag = 0;
    //Serial.println("Button released");
    bot.sendMessage("Button released!");
    led_flag = !led_flag;
    digitalWrite(2, led_flag);
  }


  if(WiFi.status() != WL_CONNECTED){             //led conect
    digitalWrite(16,HIGH);
  }
  else{
    digitalWrite(16,LOW);
  }



 bot.tick();   // тикаем в луп
}

void connectWiFi() {
  delay(2000);
  //Serial.begin(115200);
  //Serial.println();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
    if (millis() > 15000){
      ESP.restart();}
  }
    //Serial.println("Connected");
}
