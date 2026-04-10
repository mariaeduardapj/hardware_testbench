#include <Arduino.h>

#define SYSTEM_NAME "Hardware Test Bench"
#define SYSTEM_VERSION "v0.5.0"

#define BUZZER_PIN 15 
#define RELAY_PIN 5
#define ULTRASONIC_TRIGGER_PIN 19
#define ULTRASONIC_ECHO_PIN 18
#define BUTTON_PIN 21
#define LED_PIN 22
#define PIR_PIN 23

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
void ultrasonicTest() {
    Serial.print("\nStarting Ultrasonic Sensor Test... ");
    digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH); 
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    float duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);  
    float distance = (duration / 2) * 0.0343;
    Serial.print("\nUltrasonic Sensor Test completed. Distance: "); 
    Serial.print(distance);
    Serial.println(" cm");
    delay(500);
    Serial.println("Is the distance reading correct? (y/n)");
    while (Serial.available() == 0) {
    }
    char Response = Serial.read();
    if (Response == 'y' || Response == 'Y') {
      Serial.println("Ultrasonic Sensor is working correctly.");
    }
    else {
      Serial.println("Ultrasonic Sensor test failed or not confirmed.");
    }

}
void buttonledTest() {
  Serial.print("\nStarting Button and LED Test... Press the button for a few seconds to turn on the LED.");
  while (digitalRead(BUTTON_PIN) == LOW) {
  }
  if (digitalRead(BUTTON_PIN) == HIGH) {
    digitalWrite(LED_PIN, HIGH);
    Serial.print(" LED is ON. Now release the button.");
    while (digitalRead(BUTTON_PIN) == HIGH) {
    }
    digitalWrite(LED_PIN, LOW);
    Serial.println(" LED is OFF. Button and LED test completed.");
  }
  delay(1000);
  Serial.println("Is the LED turn on when the button is pressed and turn off when released? (y/n)");
  while (Serial.available() == 0) {
  }
  char Response = Serial.read();
  if (Response == 'y' || Response == 'Y') {
    Serial.println("Button and LED are working correctly.");
  }
  else {
    Serial.println("Button and LED test failed or not confirmed.");
  }
}
void pirTest() {
    Serial.println("\nRecomend do Button and LED test before this one, to make sure the LED is working correctly for this test.\nStarting PIR Sensor Test... Move your hand in front of the PIR sensor to detect motion.\nPress any key to finish the test.");
    while (Serial.available() == 0) {
        if (digitalRead(PIR_PIN) == HIGH) {
            digitalWrite(LED_PIN,HIGH);
        }
        else {
            digitalWrite(LED_PIN,LOW); 
        }
    }
   
    while (Serial.available()) {
        Serial.read();
    }
    delay(500);
    Serial.println("Is the LED turn on when the was motion detected? (y/n)");
    while (Serial.available() == 0) {
    }
    char Response = Serial.read();
    if (Response == 'y' || Response == 'Y') {
        Serial.println("PIR Sensor is working correctly.");
    }
    else {
        Serial.println("PIR Sensor test failed or not confirmed.");
    }
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

  Serial.println("Running tests...");
  Serial.println("1 - Buzzer Test\n2 - Relay Test\n3 - Ultrasonic Sensor Test\n4 - Button and LED Test\n5 - PIR Sensor Test\n6 - All Tests");
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
    else if (testNumber == 3) {
      ultrasonicTest();
    }
    else if (testNumber == 4) {
      buttonledTest();
    }
    else if (testNumber == 5) {
      pirTest();
    }
    else if (testNumber == 6) {
      buzzerTest();
      relayTest();
      ultrasonicTest();
      buttonledTest();
      pirTest();
    }
    else {
      Serial.println("Invalid test number. Please enter a valid test number.");
    }
  }
  delay(100);
}

