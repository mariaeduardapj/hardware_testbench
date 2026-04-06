#include <Arduino.h>

#define SYSTEM_NAME "Hardware Test Bench"
#define SYSTEM_VERSION "v0.1.0"
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
  if (Serial.available())  {

    String input = Serial.readStringUntil('\n'); // Read the input until a newline character is encountered
    input.trim(); // Remove any leading or trailing whitespace

    char Start = input.charAt(0); // Get the first character of the trimmed string
    if (Start == 'y' || Start == 'Y') {
      digitalWrite(BUZZER_PIN, LOW);
      delay(500);
      digitalWrite(BUZZER_PIN, HIGH);
      Serial.println("Buzzer Test completed. Start another test? (y/n)");
    }
    else if (Start == 'n' || Start == 'N') {
      Serial.println("Buzzer Test skipped. Start test now? (y/n)");
    }
    else {
      Serial.println("Invalid input. Please enter 'y' or 'n'.");
    }
  }
  delay(100);
}

