#include <Arduino.h>

#define SYSTEM_NAME "Hardware Test Bench"
#define SYSTEM_VERSION "v0.2.0"
#define BUZZER_PIN 15

void setup() {
  Serial.begin(115200);
  Serial.println("===============================================");
  Serial.println(SYSTEM_NAME " - Version " SYSTEM_VERSION);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===============================================");

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);

  Serial.println("Start Buzzer Test? (y/n)");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // Read the input until a newline character is encountered
    input.trim(); // Remove any leading or trailing whitespace

    char Start = input.charAt(0); // Get the first character of the trimmed string
    if (Start == 'y' || Start == 'Y') {
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
      while (Serial.available()) {
        Serial.read();
      }
      delay(500);
      Serial.println("Did you hear the two short beeps? (y/n)");
      while (Serial.available() == 0) {
      }
      char Response = Serial.read();
      if (Response == 'y' || Response == 'Y') {
        Serial.println("Buzzer is working correctly. Start another test? (y/n)");
      }
      else {
        Serial.println("Buzzer test failed or not confirmed. Start another test? (y/n)");
      }
    } 
    else if (Start == 'n' || Start == 'N') {
      Serial.println("Buzzer Test skipped. Start the test now? (y/n)");
    }
    else {
      Serial.println("Invalid test input. Please enter 'y' or 'n'.");
    }
  }
  delay(100);
}

