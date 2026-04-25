#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

const char *ssid = "";      // Enter the network name
const char *password = "";  // Enter the network password
AsyncWebServer server(80);

#define SYSTEM_NAME "Hardware Test Bench"
#define SYSTEM_VERSION "v1.2.0"
#define COMPATIBILITY_NOTE "Tested on ESP32 DevKit boards"

constexpr size_t MAX_DEVICES = 12;
constexpr unsigned long BUTTON_LED_TIMEOUT_MS = 10000;
constexpr unsigned long PIR_TIMEOUT_MS = 10000;

enum DeviceType {
  DEVICE_BUZZER,
  DEVICE_RELAY,
  DEVICE_ULTRASONIC,
  DEVICE_BUTTON,
  DEVICE_LED,
  DEVICE_PIR,
  DEVICE_UNKNOWN
};

struct DeviceConfig {
  bool active = false;
  uint8_t id = 0;
  DeviceType type = DEVICE_UNKNOWN;
  String name;
  int pinA = -1;
  int pinB = -1;
};

DeviceConfig devices[MAX_DEVICES];
uint8_t nextDeviceId = 1;

void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void printHeader() {
  Serial.println("\n===============================================");
  Serial.println(SYSTEM_NAME " - Version " SYSTEM_VERSION);
  Serial.println(COMPATIBILITY_NOTE);
  Serial.println("Developed by https://github.com/mariaeduardapj");
  Serial.println("===============================================\n");
}

DeviceType deviceTypeFromString(const String &type) {
  if (type == "buzzer") return DEVICE_BUZZER;
  if (type == "relay") return DEVICE_RELAY;
  if (type == "ultrasonic") return DEVICE_ULTRASONIC;
  if (type == "button") return DEVICE_BUTTON;
  if (type == "led") return DEVICE_LED;
  if (type == "pir") return DEVICE_PIR;
  return DEVICE_UNKNOWN;
}

const char *deviceTypeToString(DeviceType type) {
  switch (type) {
    case DEVICE_BUZZER:
      return "buzzer";
    case DEVICE_RELAY:
      return "relay";
    case DEVICE_ULTRASONIC:
      return "ultrasonic";
    case DEVICE_BUTTON:
      return "button";
    case DEVICE_LED:
      return "led";
    case DEVICE_PIR:
      return "pir";
    default:
      return "unknown";
  }
}

const char *deviceTypeLabel(DeviceType type) {
  switch (type) {
    case DEVICE_BUZZER:
      return "Buzzer";
    case DEVICE_RELAY:
      return "Relay";
    case DEVICE_ULTRASONIC:
      return "Ultrasonic Sensor";
    case DEVICE_BUTTON:
      return "Button";
    case DEVICE_LED:
      return "LED";
    case DEVICE_PIR:
      return "PIR Motion Sensor";
    default:
      return "Unknown";
  }
}

String jsonEscape(const String &value) {
  String escaped;
  escaped.reserve(value.length() + 8);

  // Escape user-provided strings before sending them back as JSON.
  for (size_t i = 0; i < value.length(); ++i) {
    const char ch = value[i];
    if (ch == '\\' || ch == '"') {
      escaped += '\\';
      escaped += ch;
    } else if (ch == '\n') {
      escaped += "\\n";
    } else if (ch == '\r') {
      escaped += "\\r";
    } else if (ch == '\t') {
      escaped += "\\t";
    } else {
      escaped += ch;
    }
  }

  return escaped;
}

bool isValidEsp32DevKitPin(int pin) {
  switch (pin) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 21:
    case 22:
    case 23:
    case 25:
    case 26:
    case 27:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 39:
      return true;
    default:
      return false;
  }
}

bool isOutputCapablePin(int pin) {
  return pin >= 0 && pin <= 33;
}

bool requiresSecondPin(DeviceType type) {
  return type == DEVICE_ULTRASONIC;
}

bool isPinAOutput(DeviceType type) {
  return type == DEVICE_BUZZER || type == DEVICE_RELAY || type == DEVICE_ULTRASONIC  || type == DEVICE_LED;
}

bool isPinAInput(DeviceType type) {
  return type == DEVICE_BUTTON;
}

String validatePins(DeviceType type, int pinA, int pinB) {
  if (!isValidEsp32DevKitPin(pinA)) {
    return "Primary GPIO is not valid for ESP32 DevKit.";
  }

  // Some device types drive a signal directly, so they need output-capable pins.
  if (isPinAOutput(type) && !isOutputCapablePin(pinA)) {
    return "Primary GPIO must support output for this device type.";
  }

  if (requiresSecondPin(type)) {
    if (!isValidEsp32DevKitPin(pinB)) {
      return "Secondary GPIO is not valid for ESP32 DevKit.";
    }
    if (pinA == pinB) {
      return "Primary and secondary GPIOs must be different.";
    }
    if (requiresSecondPin(type) && !isOutputCapablePin(pinB)) {
      return "Secondary GPIO must support output for this device type.";
    }
  }

  return "";
}

String validateDevicePinsAgainstRegistry(DeviceType type, int pinA, int pinB, int ignoreId = -1) {
  // Prevent two active devices from claiming the same physical GPIO.
  for (size_t i = 0; i < MAX_DEVICES; ++i) {
    if (!devices[i].active || devices[i].id == ignoreId) {
      continue;
    }

    if (devices[i].pinA == pinA || devices[i].pinB == pinA) {
      return "Primary GPIO is already assigned to another device.";
    }
    if (requiresSecondPin(type) && (devices[i].pinA == pinB || devices[i].pinB == pinB)) {
      return "Secondary GPIO is already assigned to another device.";
    }
  }

  return "";
}

void configureDevicePins(const DeviceConfig &device) {
  // Configure GPIO direction only after a device has been validated and registered.
  switch (device.type) {
    case DEVICE_BUZZER:
      pinMode(device.pinA, OUTPUT);
      digitalWrite(device.pinA, HIGH);
      break;
    case DEVICE_RELAY:
      pinMode(device.pinA, OUTPUT);
      digitalWrite(device.pinA, HIGH);
      break;
    case DEVICE_ULTRASONIC:
      pinMode(device.pinA, OUTPUT);
      digitalWrite(device.pinA, LOW);
      pinMode(device.pinB, INPUT);
      break;
    case DEVICE_BUTTON:
      pinMode(device.pinA, INPUT_PULLDOWN);
      break;
    case DEVICE_LED:
      pinMode(device.pinA, OUTPUT);
      digitalWrite(device.pinA, LOW);
      break;
    case DEVICE_PIR:
      pinMode(device.pinA, INPUT);
      break;
    default:
      break;
  }
}

DeviceConfig *findDeviceById(int id) {
  for (size_t i = 0; i < MAX_DEVICES; ++i) {
    if (devices[i].active && devices[i].id == id) {
      return &devices[i];
    }
  }

  return nullptr;
}

int findFreeDeviceSlot() {
  for (size_t i = 0; i < MAX_DEVICES; ++i) {
    if (!devices[i].active) {
      return static_cast<int>(i);
    }
  }

  return -1;
}

void successToneOnAnyBuzzer() {
  // Use the first available buzzer as optional audible feedback for web-triggered actions.
  for (size_t i = 0; i < MAX_DEVICES; ++i) {
    if (!devices[i].active || devices[i].type != DEVICE_BUZZER) {
      continue;
    }

    digitalWrite(devices[i].pinA, LOW);
    delay(70);
    digitalWrite(devices[i].pinA, HIGH);
    delay(70);
    digitalWrite(devices[i].pinA, LOW);
    delay(70);
    digitalWrite(devices[i].pinA, HIGH);
    return;
  }
}

String runBuzzerTest(const DeviceConfig &device) {
  digitalWrite(device.pinA, LOW);
  delay(80);
  digitalWrite(device.pinA, HIGH);
  delay(500);
  digitalWrite(device.pinA, LOW);
  delay(80);
  digitalWrite(device.pinA, HIGH);
  return "Buzzer test completed on GPIO " + String(device.pinA) + ".";
}

String runRelayTest(const DeviceConfig &device) {
  digitalWrite(device.pinA, LOW);
  delay(1000);
  digitalWrite(device.pinA, HIGH);
  return "Relay test completed on GPIO " + String(device.pinA) + ".";
}

String runUltrasonicTest(const DeviceConfig &device) {
  digitalWrite(device.pinA, LOW);
  delayMicroseconds(2);
  digitalWrite(device.pinA, HIGH);
  delayMicroseconds(10);
  digitalWrite(device.pinA, LOW);

  unsigned long duration = pulseIn(device.pinB, HIGH, 30000);
  if (duration == 0) {
    return "Ultrasonic reading timed out. Check trigger/echo wiring.";
  }

  float distance = (duration / 2.0F) * 0.0343F;
  return "Distance measured: " + String(distance, 2) + " cm.";
}

String runButtonTest(const DeviceConfig &device) {
  unsigned long startedAt = millis();
  // Wait for a real button press, but avoid blocking forever.
  while (digitalRead(device.pinA) == LOW) {
    if (millis() - startedAt > BUTTON_LED_TIMEOUT_MS) {
      return "Button test timed out. Press the button and try again.";
    }
    delay(10);
  }
  return "Button press detected successfully.";
}

String runLedTest(const DeviceConfig &device) {
  digitalWrite(device.pinA, HIGH);
  delay(500);
  digitalWrite(device.pinA, LOW);
  return "LED test completed on GPIO " + String(device.pinA) + ".";
}

String runPirTest(const DeviceConfig &device) {
  unsigned long startedAt = millis();
  // PIR sensors are passive inputs, so this test only waits for a motion event.
  while (millis() - startedAt <= PIR_TIMEOUT_MS) {
    if (digitalRead(device.pinA) == HIGH) {
      return "Motion detected on GPIO " + String(device.pinA) + ".";
    }
    delay(50);
  }

  return "No motion detected before timeout.";
}

String executeTest(const DeviceConfig &device) {
  Serial.println("Running device test: " + device.name + " [" + String(deviceTypeToString(device.type)) + "]");

  switch (device.type) {
    case DEVICE_BUZZER:
      return runBuzzerTest(device);
    case DEVICE_RELAY:
      return runRelayTest(device);
    case DEVICE_ULTRASONIC:
      return runUltrasonicTest(device);
    case DEVICE_BUTTON:
      return runButtonTest(device);
    case DEVICE_LED:
      return runLedTest(device);
    case DEVICE_PIR:
      return runPirTest(device);
    default:
      return "Unsupported device type.";
  }
}

String buildCatalogJson() {
  // The frontend uses this catalog to build the form dynamically.
  return String("{") +
         "\"systemName\":\"" + String(SYSTEM_NAME) + "\"," +
         "\"version\":\"" + String(SYSTEM_VERSION) + "\"," +
         "\"compatibility\":\"" + String(COMPATIBILITY_NOTE) + "\"," +
         "\"supportedBoards\":[\"ESP32 DevKit\"]," +
         "\"deviceTypes\":[" +
         "{\"id\":\"buzzer\",\"label\":\"Buzzer\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"]}," +
         "{\"id\":\"relay\",\"label\":\"Relay\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"]}," +
         "{\"id\":\"ultrasonic\",\"label\":\"Ultrasonic Sensor\",\"pins\":2,\"pinLabels\":[\"Trigger GPIO\",\"Echo GPIO\"]}," +
         "{\"id\":\"button\",\"label\":\"Button\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"]}," +
         "{\"id\":\"led\",\"label\":\"LED\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"]}," +
         "{\"id\":\"pir\",\"label\":\"PIR Motion Sensor\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"]}" +
         "]" +
         "}";
}

String buildDevicesJson() {
  String json = "[";
  bool first = true;

  // Return only active device slots so the UI can render the current bench state.
  for (size_t i = 0; i < MAX_DEVICES; ++i) {
    if (!devices[i].active) {
      continue;
    }

    if (!first) {
      json += ",";
    }
    first = false;

    json += "{";
    json += "\"id\":" + String(devices[i].id) + ",";
    json += "\"name\":\"" + jsonEscape(devices[i].name) + "\",";
    json += "\"type\":\"" + String(deviceTypeToString(devices[i].type)) + "\",";
    json += "\"typeLabel\":\"" + String(deviceTypeLabel(devices[i].type)) + "\",";
    json += "\"pinA\":" + String(devices[i].pinA) + ",";
    json += "\"pinB\":" + String(devices[i].pinB) + ",";
    json += "\"hasSecondPin\":" + String(requiresSecondPin(devices[i].type) ? "true" : "false");
    json += "}";
  }

  json += "]";
  return json;
}

String requestParamValue(AsyncWebServerRequest *request, const char *name) {
  if (!request->hasParam(name)) {
    return "";
  }
  return request->getParam(name)->value();
}

void setupRoutes() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/script.js", "text/javascript");
  });

  server.on("/supported.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/supported.html", "text/html");
  });

  server.on("/supported.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/supported.js", "text/javascript");
  });

  server.on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/favicon.png", "image/png");
  });

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    String info = String(SYSTEM_NAME) + " " + String(SYSTEM_VERSION) + " | " + String(COMPATIBILITY_NOTE);
    request->send(200, "text/plain", info);
  });

  server.on("/catalog", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildCatalogJson());
  });

  server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", buildDevicesJson());
  });

  server.on("/devices/add", HTTP_POST, [](AsyncWebServerRequest *request) {
    const String typeRaw = requestParamValue(request, "type");
    const String name = requestParamValue(request, "name");
    const int pinA = requestParamValue(request, "pinA").toInt();
    const int pinB = requestParamValue(request, "pinB").toInt();

    // Register a new device instance based on the type selected in the web UI.
    DeviceType type = deviceTypeFromString(typeRaw);
    if (type == DEVICE_UNKNOWN) {
      request->send(400, "text/plain", "Unsupported device type.");
      return;
    }

    if (name.length() == 0) {
      request->send(400, "text/plain", "Device name is required.");
      return;
    }

    String pinError = validatePins(type, pinA, pinB);
    if (pinError.length() > 0) {
      request->send(400, "text/plain", pinError);
      return;
    }

    String pinConflict = validateDevicePinsAgainstRegistry(type, pinA, pinB);
    if (pinConflict.length() > 0) {
      request->send(400, "text/plain", pinConflict);
      return;
    }

    int slot = findFreeDeviceSlot();
    if (slot < 0) {
      request->send(400, "text/plain", "Device limit reached.");
      return;
    }

    devices[slot].active = true;
    devices[slot].id = nextDeviceId++;
    devices[slot].type = type;
    devices[slot].name = name;
    devices[slot].pinA = pinA;
    devices[slot].pinB = requiresSecondPin(type) ? pinB : -1;
    configureDevicePins(devices[slot]);

    Serial.println("Device registered: " + devices[slot].name + " (" + String(deviceTypeToString(type)) + ")");
    request->send(200, "text/plain", "Device added successfully.");
  });

  server.on("/devices/remove", HTTP_POST, [](AsyncWebServerRequest *request) {
    const int id = requestParamValue(request, "id").toInt();
    DeviceConfig *device = findDeviceById(id);

    if (device == nullptr) {
      request->send(404, "text/plain", "Device not found.");
      return;
    }

    Serial.println("Removing device: " + device->name);
    *device = DeviceConfig();
    request->send(200, "text/plain", "Device removed.");
  });

  server.on("/run", HTTP_POST, [](AsyncWebServerRequest *request) {
    const int id = requestParamValue(request, "id").toInt();
    DeviceConfig *device = findDeviceById(id);

    if (device == nullptr) {
      request->send(404, "text/plain", "Device not found.");
      return;
    }

    // Execute the test against one configured device instead of a global fixed pin map.
    successToneOnAnyBuzzer();
    String result = executeTest(*device);
    request->send(200, "text/plain", result);
  });
}

void setup() {
  Serial.begin(115200);
  printHeader();

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS error");
    return;
  }

  initWiFi();
  setupRoutes();
  server.begin();
}

void loop() {
  // All operations are handled by the async web server callbacks.
}
