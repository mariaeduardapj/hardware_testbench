#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <WiFi.h>

const char *ssid = ""; // Change this to your WiFi network name
const char *password = ""; // Change this to your WiFi password
AsyncWebServer server(80);

#define SYSTEM_NAME "Hardware Test Bench"
#define SYSTEM_VERSION "v1.2.1"
#define COMPATIBILITY_NOTE "Tested on ESP32 DevKit boards"

constexpr size_t MAX_DEVICES = 12;
constexpr unsigned long BUTTON_LED_TIMEOUT_MS = 10000;
constexpr unsigned long PIR_TIMEOUT_MS = 10000;
constexpr unsigned long OUTPUT_PULSE_MS = 500;

enum DeviceType {
  DEVICE_BUZZER,
  DEVICE_RELAY,
  DEVICE_ULTRASONIC,
  DEVICE_BUTTON,
  DEVICE_LED,
  DEVICE_PIR,
  DEVICE_UNKNOWN
};

enum ValidationState {
  VALIDATION_PENDING,
  VALIDATION_PASS,
  VALIDATION_FAIL
};

enum TestStatus {
  TEST_IDLE,
  TEST_RUNNING,
  TEST_COMPLETED
};

enum TestPhase {
  PHASE_IDLE,
  PHASE_WAITING_INPUT,
  PHASE_OUTPUT_ACTIVE
};

struct TestState {
  TestStatus status = TEST_IDLE;
  TestPhase phase = PHASE_IDLE;
  ValidationState suggestedResult = VALIDATION_PENDING;
  ValidationState finalResult = VALIDATION_PENDING;
  bool awaitingConfirmation = false;
  bool userCanOverride = false;
  unsigned long startedAtMs = 0;
  unsigned long timestampMs = 0;
  unsigned long stepStartedAtMs = 0;
  String message = "Ready to run.";
};

struct DeviceConfig {
  bool active = false;
  uint8_t id = 0;
  DeviceType type = DEVICE_UNKNOWN;
  String name;
  int pinA = -1;
  int pinB = -1;
  TestState testState;
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

const char *validationStateToString(ValidationState state) {
  switch (state) {
    case VALIDATION_PASS:
      return "PASS";
    case VALIDATION_FAIL:
      return "FAIL";
    default:
      return "PENDING";
  }
}

const char *testStatusToString(TestStatus status) {
  switch (status) {
    case TEST_RUNNING:
      return "Running";
    case TEST_COMPLETED:
      return "Completed";
    default:
      return "Idle";
  }
}

String jsonEscape(const String &value) {
  String escaped;
  escaped.reserve(value.length() + 8);

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
  return type == DEVICE_BUZZER || type == DEVICE_RELAY || type == DEVICE_ULTRASONIC || type == DEVICE_LED;
}

String validatePins(DeviceType type, int pinA, int pinB) {
  if (!isValidEsp32DevKitPin(pinA)) {
    return "Primary GPIO is not valid for ESP32 DevKit.";
  }

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
  }

  return "";
}

String validateDevicePinsAgainstRegistry(DeviceType type, int pinA, int pinB, int ignoreId = -1) {
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
  for (size_t i = 0; i < MAX_DEVICES; ++i) {
    if (!devices[i].active || devices[i].type != DEVICE_BUZZER) {
      continue;
    }

    digitalWrite(devices[i].pinA, LOW);
    delay(40);
    digitalWrite(devices[i].pinA, HIGH);
    return;
  }
}

bool usesHybridConfirmation(DeviceType type) {
  return type == DEVICE_BUZZER || type == DEVICE_RELAY || type == DEVICE_LED;
}

void resetTestState(DeviceConfig &device) {
  device.testState = TestState();
}

void completeTest(DeviceConfig &device, ValidationState suggestedResult, const String &message, bool awaitingConfirmation) {
  device.testState.status = TEST_COMPLETED;
  device.testState.phase = PHASE_IDLE;
  device.testState.suggestedResult = suggestedResult;
  device.testState.finalResult = awaitingConfirmation ? VALIDATION_PENDING : suggestedResult;
  device.testState.awaitingConfirmation = awaitingConfirmation;
  device.testState.userCanOverride = true;
  device.testState.timestampMs = millis();
  device.testState.message = message;
}

void startTest(DeviceConfig &device) {
  device.testState.status = TEST_RUNNING;
  device.testState.phase = PHASE_IDLE;
  device.testState.suggestedResult = VALIDATION_PENDING;
  device.testState.finalResult = VALIDATION_PENDING;
  device.testState.awaitingConfirmation = false;
  device.testState.userCanOverride = false;
  device.testState.startedAtMs = millis();
  device.testState.stepStartedAtMs = millis();
  device.testState.timestampMs = 0;
  device.testState.message = "Test started.";

  switch (device.type) {
    case DEVICE_BUTTON:
    case DEVICE_PIR:
      device.testState.phase = PHASE_WAITING_INPUT;
      device.testState.message = "Waiting for input event...";
      break;
    case DEVICE_LED:
      digitalWrite(device.pinA, HIGH);
      device.testState.phase = PHASE_OUTPUT_ACTIVE;
      device.testState.message = "LED turned on. Suggested result will be PASS. Please confirm what you saw.";
      break;
    case DEVICE_RELAY:
      digitalWrite(device.pinA, LOW);
      device.testState.phase = PHASE_OUTPUT_ACTIVE;
      device.testState.message = "Relay activated. Suggested result will be PASS. Please confirm the click/output.";
      break;
    case DEVICE_BUZZER:
      digitalWrite(device.pinA, LOW);
      device.testState.phase = PHASE_OUTPUT_ACTIVE;
      device.testState.message = "Buzzer activated. Suggested result will be PASS. Please confirm the sound.";
      break;
    case DEVICE_ULTRASONIC:
      digitalWrite(device.pinA, LOW);
      delayMicroseconds(2);
      digitalWrite(device.pinA, HIGH);
      delayMicroseconds(10);
      digitalWrite(device.pinA, LOW);
      {
        unsigned long duration = pulseIn(device.pinB, HIGH, 30000);
        if (duration == 0) {
          completeTest(device, VALIDATION_FAIL, "Ultrasonic reading timed out. Check trigger/echo wiring.", false);
        } else {
          float distance = (duration / 2.0F) * 0.0343F;
          completeTest(device, VALIDATION_PASS, "Distance measured: " + String(distance, 2) + " cm.", false);
        }
      }
      break;
    default:
      completeTest(device, VALIDATION_FAIL, "Unsupported device type.", false);
      break;
  }
}

void updateRunningTest(DeviceConfig &device) {
  if (!device.active || device.testState.status != TEST_RUNNING) {
    return;
  }

  const unsigned long now = millis();

  switch (device.type) {
    case DEVICE_PIR:
      if (digitalRead(device.pinA) == HIGH) {
        completeTest(device, VALIDATION_PASS, "Motion detected -> PASS. Confirm result?", true);
      } else if (now - device.testState.startedAtMs >= PIR_TIMEOUT_MS) {
        completeTest(device, VALIDATION_FAIL, "No motion detected within 10 seconds -> FAIL. Confirm result?", true);
      }
      break;

    case DEVICE_BUTTON:
      if (digitalRead(device.pinA) == HIGH) {
        completeTest(device, VALIDATION_PASS, "Button press detected -> PASS. Confirm result?", true);
      } else if (now - device.testState.startedAtMs >= BUTTON_LED_TIMEOUT_MS) {
        completeTest(device, VALIDATION_FAIL, "No button press detected within 10 seconds -> FAIL. Confirm result?", true);
      }
      break;

    case DEVICE_LED:
      if (device.testState.phase == PHASE_OUTPUT_ACTIVE && now - device.testState.stepStartedAtMs >= OUTPUT_PULSE_MS) {
        digitalWrite(device.pinA, LOW);
        completeTest(device, VALIDATION_PASS, "LED pulse completed -> PASS suggested. Confirm if the LED lit correctly.", true);
      }
      break;

    case DEVICE_RELAY:
      if (device.testState.phase == PHASE_OUTPUT_ACTIVE && now - device.testState.stepStartedAtMs >= OUTPUT_PULSE_MS) {
        digitalWrite(device.pinA, HIGH);
        completeTest(device, VALIDATION_PASS, "Relay pulse completed -> PASS suggested. Confirm if the relay switched correctly.", true);
      }
      break;

    case DEVICE_BUZZER:
      if (device.testState.phase == PHASE_OUTPUT_ACTIVE && now - device.testState.stepStartedAtMs >= OUTPUT_PULSE_MS) {
        digitalWrite(device.pinA, HIGH);
        completeTest(device, VALIDATION_PASS, "Buzzer pulse completed -> PASS suggested. Confirm if the sound was heard.", true);
      }
      break;

    default:
      break;
  }
}

void updateRunningTests() {
  for (size_t i = 0; i < MAX_DEVICES; ++i) {
    updateRunningTest(devices[i]);
  }
}

String buildCatalogJson() {
  return String("{") +
         "\"systemName\":\"" + String(SYSTEM_NAME) + "\"," +
         "\"version\":\"" + String(SYSTEM_VERSION) + "\"," +
         "\"compatibility\":\"" + String(COMPATIBILITY_NOTE) + "\"," +
         "\"supportedBoards\":[\"ESP32 DevKit\"]," +
         "\"deviceTypes\":[" +
         "{\"id\":\"buzzer\",\"label\":\"Buzzer\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"],\"hybridValidation\":true}," +
         "{\"id\":\"relay\",\"label\":\"Relay\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"],\"hybridValidation\":true}," +
         "{\"id\":\"ultrasonic\",\"label\":\"Ultrasonic Sensor\",\"pins\":2,\"pinLabels\":[\"Trigger GPIO\",\"Echo GPIO\"],\"hybridValidation\":false}," +
         "{\"id\":\"button\",\"label\":\"Button\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"],\"hybridValidation\":true}," +
         "{\"id\":\"led\",\"label\":\"LED\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"],\"hybridValidation\":true}," +
         "{\"id\":\"pir\",\"label\":\"PIR Motion Sensor\",\"pins\":1,\"pinLabels\":[\"Signal GPIO\"],\"hybridValidation\":true}" +
         "]" +
         "}";
}

String buildTestStateJson(const TestState &testState) {
  return String("{") +
         "\"status\":\"" + String(testStatusToString(testState.status)) + "\"," +
         "\"suggestedResult\":\"" + String(validationStateToString(testState.suggestedResult)) + "\"," +
         "\"finalResult\":\"" + String(validationStateToString(testState.finalResult)) + "\"," +
         "\"message\":\"" + jsonEscape(testState.message) + "\"," +
         "\"timestampMs\":" + String(testState.timestampMs) + "," +
         "\"awaitingConfirmation\":" + String(testState.awaitingConfirmation ? "true" : "false") + "," +
         "\"userCanOverride\":" + String(testState.userCanOverride ? "true" : "false") +
         "}";
}

String buildDevicesJson() {
  String json = "[";
  bool first = true;

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
    json += "\"hasSecondPin\":" + String(requiresSecondPin(devices[i].type) ? "true" : "false") + ",";
    json += "\"hybridValidation\":" + String(usesHybridConfirmation(devices[i].type) ? "true" : "false") + ",";
    json += "\"test\":" + buildTestStateJson(devices[i].testState);
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
    resetTestState(devices[slot]);
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

  server.on("/run/start", HTTP_POST, [](AsyncWebServerRequest *request) {
    const int id = requestParamValue(request, "id").toInt();
    DeviceConfig *device = findDeviceById(id);

    if (device == nullptr) {
      request->send(404, "text/plain", "Device not found.");
      return;
    }

    if (device->testState.status == TEST_RUNNING) {
      request->send(400, "text/plain", "A test is already running for this device.");
      return;
    }

    successToneOnAnyBuzzer();
    startTest(*device);
    Serial.println("Running device test: " + device->name + " [" + String(deviceTypeToString(device->type)) + "]");
    request->send(200, "application/json", buildTestStateJson(device->testState));
  });

  server.on("/run/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    const int id = requestParamValue(request, "id").toInt();
    DeviceConfig *device = findDeviceById(id);

    if (device == nullptr) {
      request->send(404, "text/plain", "Device not found.");
      return;
    }

    request->send(200, "application/json", buildTestStateJson(device->testState));
  });

  server.on("/run/finalize", HTTP_POST, [](AsyncWebServerRequest *request) {
    const int id = requestParamValue(request, "id").toInt();
    const String resultRaw = requestParamValue(request, "result");
    DeviceConfig *device = findDeviceById(id);

    if (device == nullptr) {
      request->send(404, "text/plain", "Device not found.");
      return;
    }

    if (device->testState.status != TEST_COMPLETED) {
      request->send(400, "text/plain", "There is no completed test to confirm.");
      return;
    }

    ValidationState chosenResult = VALIDATION_PENDING;
    if (resultRaw == "PASS") {
      chosenResult = VALIDATION_PASS;
    } else if (resultRaw == "FAIL") {
      chosenResult = VALIDATION_FAIL;
    } else {
      request->send(400, "text/plain", "Result must be PASS or FAIL.");
      return;
    }

    device->testState.finalResult = chosenResult;
    device->testState.awaitingConfirmation = false;
    device->testState.userCanOverride = true;
    device->testState.timestampMs = millis();

    if (chosenResult == device->testState.suggestedResult) {
      device->testState.message += " User confirmed suggested result.";
    } else {
      device->testState.message += " User override applied.";
    }

    request->send(200, "application/json", buildTestStateJson(device->testState));
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
  updateRunningTests();
}
