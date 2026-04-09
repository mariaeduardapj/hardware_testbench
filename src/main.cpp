#include <Arduino.h>

#define SYSTEM_NAME "Hardware Test Bench"
#define SYSTEM_VERSION "v0.3.0"

#define BUZZER_PIN 15 
#define RELAY_PIN 5

void printHeader() {
  Serial.println("\n===============================================");
  Serial.println(SYSTEM_NAME " - Version " SYSTEM_VERSION);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===============================================\n");
}

void buzzerTest() {
  Serial.print("\nStarting Buzzer Test... ");
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
  delay(50);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  Serial.println("Buzzer Test completed.");
  delay(500);
  Serial.println("Did you hear the two short beeps? (y/n)");
  while (Serial.available() == 0) {
  }
  char Response = Serial.read();
  if (Response == 'y' || Response == 'Y') {
    Serial.println("Buzzer is working correctly.");
  }
  else {
    Serial.println("Buzzer test failed or not confirmed.");
  }
}
void relayTest() {
  Serial.print("\nStarting Relay Test... ");
  digitalWrite(RELAY_PIN, LOW);
  delay(1000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(1000);
  Serial.println("Relay Test completed.");
  delay(500);
  Serial.println("Did you hear the relay click? (y/n)");
  while (Serial.available() == 0) {
  }
  char Response = Serial.read();
  if (Response == 'y' || Response == 'Y') {
    Serial.println("Relay is working correctly.");
  }
  else {
    Serial.println("Relay test failed or not confirmed.");
  }
}

void setup() {
  Serial.begin(115200);
  printHeader();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  Serial.println("Running tests...");
  Serial.println("1 - Buzzer Test\n2 - Relay Test");
}

void loop() {
  if (Serial.available())  {

    String input = Serial.readStringUntil('\n');
    input.trim();

    int testNumber = input.toInt();
    if (testNumber == 1) {
      buzzerTest();
    }
    else if (testNumber == 2) {
      relayTest();
    }
    else {
      Serial.println("Invalid test number. Please enter a valid test number.");
    }
  }
  delay(100);
}

