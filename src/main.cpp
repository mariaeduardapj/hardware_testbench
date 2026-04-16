#include <Arduino.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
const char* ssid = ""; // Enter the network name
const char* password = ""; // Enter the network password
AsyncWebServer server(80);

#define SYSTEM_NAME "Hardware Test Bench"
#define SYSTEM_VERSION "v1.1.0"

#define BUZZER_PIN 15 
#define RELAY_PIN 5
#define ULTRASONIC_TRIGGER_PIN 19
#define ULTRASONIC_ECHO_PIN 18
#define BUTTON_PIN 21
#define LED_PIN 22
#define PIR_PIN 23

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Try connecting to local WiFi

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void printHeader() {
  Serial.println("\n===============================================");
  Serial.println(SYSTEM_NAME " - Version " SYSTEM_VERSION);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===============================================\n");
}
void buzzerTest() {
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
}
void relayTest() {
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
}
float ultrasonicTest() {
    digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH); 
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    float duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);  
    float distance = (duration / 2) * 0.0343;
    return distance;
}
void buttonledTest() {
  while (digitalRead(BUTTON_PIN) == LOW) {
  }
  if (digitalRead(BUTTON_PIN) == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    while (digitalRead(BUTTON_PIN) == HIGH) {
    }
    digitalWrite(LED_PIN, LOW);
  }
}
void pirTest() {  
  while (true) {
    if (digitalRead(PIR_PIN) == HIGH) {
      digitalWrite(LED_PIN,HIGH);
      delay(1000); 
      digitalWrite(LED_PIN,LOW);  
      break;
      }
    else {
      digitalWrite(LED_PIN,LOW); 
      }
    }
}
void playSucessTone() {
  digitalWrite(BUZZER_PIN, LOW);
  delay(100);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
}

void setup() {
  Serial.begin(115200);
  printHeader();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);

  // Start LittleFS and Pins
  if(!LittleFS.begin(true)) { Serial.println("LittleFS Error"); return; }
  initWiFi();

  // Serving static files
  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request){
    String info = String(SYSTEM_NAME) + " " + String(SYSTEM_VERSION);
    request->send(200, "text/plain", info);
  });
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/script.js", "text/javascript");
  });

  server.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/favicon.png", "image/png");
  });

  server.on("/ultrasonic", HTTP_GET, [](AsyncWebServerRequest *request){
    playSucessTone();
    float distance = ultrasonicTest();
    
    // Converts the float to a string to sending via http  
    request->send(200, "text/plain", String(distance));
});

  // Test master route
  server.on("/run", HTTP_GET, [](AsyncWebServerRequest *request){
    playSucessTone();
    if (request->hasParam("test")) {
      String test = request->getParam("test")->value();
      Serial.println("Running via Web: " + test);

      if (test == "buzzer") buzzerTest();
      else if (test == "relay") relayTest();
      else if (test == "ultrasonic") ultrasonicTest();
      else if (test == "buttonled") buttonledTest();
      else if (test == "pir") pirTest();

      request->send(200, "text/plain", "Test " + test + " completed!");
    } else {
      request->send(400, "text/plain", "Missing parameter");
    }
  });

  server.begin();
}

void loop() {
  // Principal loop is empty since all operations are handled via web server callbacks
}

