#define ver "version 0.6"


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <FastBot.h> 
#include <Adafruit_NeoPixel.h>

#define BUTTON_TRIGER_PIN 16       // Define the button pin that is the electricity sensor
#define BUTTON_RESET_PIN 5        // Define the factory reset button pin

#define LED_PIN 2                 // Define the LED pin
#define LED_WIFI_PIN 4           // Define the LED pin which indicates the status: when the LED is on it means the esp8266 is successfully connected to the WiFi, when the LED blinks 2 times per second it means that the esp8266 has become an access point
#define PIXEL_ADDRESS 0
#define NUMPIXELS 1

#define BUTTON_PRESS_TIME 10000   // 10 seconds in milliseconds
#define LED_BLINK_INTERVAL 500    // LED blink interval for AP mode (milliseconds)
#define AP_TIMEOUT 60000          // 1 minute in milliseconds



#define BOT_TOKEN "xxxxxxxxxx:xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
#define CHAT_ID "xxxxxxxxx"


Adafruit_NeoPixel pixels(NUMPIXELS, LED_WIFI_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);
FastBot bot(BOT_TOKEN);


const char* apSSID = "Electrotropium";
const char* apPassword = "config123";

unsigned long buttonPressStartTime = 0;
bool buttonHeld = false;
unsigned long lastBlinkTime = 0;
unsigned long nowBlinkTime;

bool address_flag_wifi = false;
bool address_flag_ap = false;
bool address_flag_ap_connected = false;

unsigned long apStartTime = 0;

bool butt_flag = 1;
bool butt;
bool led_flag = 0;
unsigned long last_press;


uint32_t startUnix;     // save time


void clearEEPROM() {
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.end();
}

void setup() {
  Serial.begin(115200);

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)

  // Setup the button pin as input with an internal pull-up resistor
  pinMode(BUTTON_TRIGER_PIN, INPUT);
  pinMode(BUTTON_RESET_PIN, INPUT);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_WIFI_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);
 // digitalWrite(LED_WIFI_PIN, HIGH);




  // Initialize EEPROM
  EEPROM.begin(512);

  // Attempt to connect to the stored WiFi credentials
  WiFi.mode(WIFI_STA);
  String ssid = "", password = "";

  // Read SSID from EEPROM
  for (int i = 0; i < 32; ++i) {
    char c = EEPROM.read(i);
    if (c == '\0') break;
    ssid += c;
  }

  // Read password from EEPROM
  for (int i = 0; i < 64; ++i) {
    char c = EEPROM.read(100 + i);
    if (c == '\0') break;
    password += c;
  }

  // Connect to WiFi if credentials are available
  if (ssid.length() > 0 && password.length() > 0) {
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi");
    int maxAttempts = 20;  // Max attempts to connect before giving up
    int attempts = 0;
    
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
//      digitalWrite(LED_WIFI_PIN, HIGH);  // Turn on LED when connected to WiFi


      bot.setChatID(CHAT_ID);
      bot.attach(newMsg);

      bot.setTextMode(FB_MARKDOWN);  
      bot.sendMessage("*Electrotropium*");
      bot.sendMessage(ver);
      bot.setTextMode(FB_TEXT);  

      startUnix = bot.getUnix();


      return;  // If connected, no need to start the AP mode
    } else {
      Serial.println("\nFailed to connect to WiFi.");
    }
  } else {
    Serial.println("No stored credentials.");
  }

  // If not connected to WiFi, start Access Point mode
  WiFi.mode(WIFI_AP);
  bool apStarted = WiFi.softAP(apSSID, apPassword, 1, false, 8);  // Channel 1, hidden SSID false, max connections 8
  if (apStarted) {
    Serial.println("AP mode started successfully");
    Serial.print("AP IP Address: ");
    Serial.println(WiFi.softAPIP());
    apStartTime = millis();  // Start the AP timeout timer
  } else {
    Serial.println("Failed to start AP mode");
  }

  // Set up the web server
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
  Serial.println("Web server started");
}


// message handler
void newMsg(FB_msg& msg) {
  if (msg.unix < startUnix) return; // ignor massenge

  if (msg.text == "/status" && led_flag == 1) {
    bot.sendMessage("There are no electrics", msg.chatID);
  }

  if (msg.text == "/status" && led_flag == 0) {
    bot.sendMessage("There are electrics", msg.chatID);
  }
}



void handleRoot() {
  String html = "<html><body><h1>Electrotropium config</h1>";
  html += "<form action='/save' method='POST'>";
  html += "SSID: <input type='text' name='ssid'><br>";
  html += "Password: <input type='password' name='password'><br>";
  html += "<input type='submit' value='Save'>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSave() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  EEPROM.begin(512);
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.write(ssid.length(), '\0');  // Null-terminate the SSID

  for (int i = 0; i < password.length(); ++i) {
    EEPROM.write(100 + i, password[i]);
  }
  EEPROM.write(100 + password.length(), '\0');  // Null-terminate the password

  EEPROM.commit();

  server.send(200, "text/html", "<html><body><h1>Credentials Saved! Rebooting...</h1></body></html>");
  delay(2000);
  ESP.restart();
}

void loop() {
  server.handleClient();

  // Button triger Handling Logic
  butt = digitalRead(BUTTON_TRIGER_PIN);
  
  if (butt == 1 && butt_flag == 0 && millis() - last_press > 100) {
    butt_flag = 1;
    Serial.println("The electricity has just appeared");
    bot.sendMessage("The electricity has just appeared");
    led_flag = !led_flag;
    digitalWrite(LED_PIN, led_flag);
    last_press = millis();
  }

  if (butt == 0 && butt_flag == 1) {
    butt_flag = 0;
    Serial.println("The electricity has just disappeared");
    bot.sendMessage("The electricity has just disappeared");
    led_flag = !led_flag;
    digitalWrite(LED_PIN, led_flag);
  }


  // Button factory reset Handling Logic
  if (digitalRead(BUTTON_RESET_PIN) == HIGH) {  // Button pressed
    if (!buttonHeld) {
      buttonHeld = true;
      buttonPressStartTime = millis();  // Start timing
    } else {
      // Check if the button has been pressed for 10 seconds
      if (millis() - buttonPressStartTime >= BUTTON_PRESS_TIME) {
        Serial.println("Button held for 10 seconds. Clearing WiFi credentials...");
        bot.sendMessage("Someone killed me");
        clearEEPROM();
        delay(1000);  // Small delay before restarting
        ESP.restart();
      }
    }
  } else {  // Button released
    buttonHeld = false;  // Reset the state when the button is released
  }


  

  // LED Handling Logic
  if (WiFi.status() != WL_CONNECTED) {  // AP mode active
   // if (millis() - lastBlinkTime >= LED_BLINK_INTERVAL) {
    //  ledState = !ledState;  // Toggle LED state  `
     // digitalWrite(LED_WIFI_PIN, ledState ? HIGH : LOW);  // Blink LED
    //  lastBlinkTime = millis();  // Reset blink timer
    //}

    // Check if any station has connected to the AP
    if (WiFi.softAPgetStationNum() == 0 && millis() - apStartTime >= AP_TIMEOUT) {
      Serial.println("No device connected to the AP within 1 minute. Retrying WiFi connection...");
      WiFi.softAPdisconnect(true);  // Turn off the AP mode
      delay(1000);
      setup();  // No restart, but reset the ESP to re-connecting to WiFi
    }
  }



 if (WiFi.status() == WL_CONNECTED) {
      if(butt == 1 && address_flag_wifi == 0){
      pixels.clear(); // Set all pixel colors to 'off'
      pixels.setPixelColor(PIXEL_ADDRESS, pixels.Color(0, 175, 0));
      address_flag_wifi = 1;
      pixels.show();
      }

      if(butt == 0 && address_flag_wifi == 1){
      pixels.clear(); // Set all pixel colors to 'off'
      pixels.setPixelColor(PIXEL_ADDRESS, pixels.Color(0, 15, 0));
      address_flag_wifi = 0;
     pixels.show();

      Serial.print("bip-bip-bip");
      }
    
  }

 if (WiFi.status() != WL_CONNECTED && WiFi.softAPgetStationNum() == 0) {
      if(butt == 1 && address_flag_ap == 0){
      pixels.clear(); // Set all pixel colors to 'off'
      pixels.setPixelColor(PIXEL_ADDRESS, pixels.Color(200, 0, 0));
      address_flag_ap = 1;
      pixels.show();
      }

      if(butt == 0 && address_flag_ap == 1){
      pixels.clear(); // Set all pixel colors to 'off'
      pixels.setPixelColor(PIXEL_ADDRESS, pixels.Color(15, 0, 0));
      address_flag_ap = 0;
      pixels.show();
      }
 }

 if (WiFi.status() != WL_CONNECTED && WiFi.softAPgetStationNum() > 0) {
  nowBlinkTime = millis();  // Текущее время в миллисекундах
  // Проверяем, прошел ли интервал времени для мигания светодиода
  if (millis() - lastBlinkTime >= LED_BLINK_INTERVAL) {
    lastBlinkTime = nowBlinkTime;  // Обновляем время последнего изменения состояния

    // Меняем состояние светодиода
    address_flag_ap_connected = !address_flag_ap_connected;
    
    if (address_flag_ap_connected) {
      pixels.setPixelColor(PIXEL_ADDRESS, pixels.Color(200, 0, 0));  // Включаем светодиод (красный цвет)
    } 
    else {
      pixels.setPixelColor(PIXEL_ADDRESS, pixels.Color(0, 0, 0));    // Выключаем светодиод
    }
    
    pixels.show();  // Обновляем светодиод
  }



  }


  bot.tick();
}
