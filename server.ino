#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <DabbleESP32.h>

// ===== WiFi Credentials =====
const char* ssid = "najil";
const char* password = "project123";
WiFiServer server(80);

// ===== Motor Driver Pins (L298N) =====
const int enableA = 5, motorLeft1 = 19, motorLeft2 = 21;
const int enableB = 18, motorRight1 = 22, motorRight2 = 23;

// ===== Conveyor Belt Pins (L298N) =====
const int enableBeltA = 33, beltLeft1 = 25, beltLeft2 = 26;
const int enableBeltB = 4, beltRight1 = 27, beltRight2 = 32;

bool processingWaste = false;

// ===== Mode Selection =====
enum Mode { AUTO_MODE, MANUAL_MODE };
Mode currentMode = AUTO_MODE;

// ===== Motor State Tracking =====
bool motorsStopped = true;

// ===== WiFi Reconnect Management =====
unsigned long lastWifiReconnectAttempt = 0;
const unsigned long wifiReconnectInterval = 10000; // 10 seconds

// ---------------- Motor Functions ----------------
void moveForward() {
  digitalWrite(enableA, HIGH); digitalWrite(enableB, HIGH);
  digitalWrite(motorLeft1, HIGH); digitalWrite(motorLeft2, LOW);
  digitalWrite(motorRight1, HIGH); digitalWrite(motorRight2, LOW);
  if (motorsStopped) {
    Serial.println("🚗 Moving Forward");
    motorsStopped = false;
  }
}

void moveBackward() {
  digitalWrite(enableA, HIGH); digitalWrite(enableB, HIGH);
  digitalWrite(motorLeft1, LOW); digitalWrite(motorLeft2, HIGH);
  digitalWrite(motorRight1, LOW); digitalWrite(motorRight2, HIGH);
  if (motorsStopped) {
    Serial.println("🚗 Moving Backward");
    motorsStopped = false;
  }
}

void moveLeft() {
  digitalWrite(enableA, HIGH); digitalWrite(enableB, HIGH);
  digitalWrite(motorLeft1, LOW); digitalWrite(motorLeft2, HIGH);
  digitalWrite(motorRight1, HIGH); digitalWrite(motorRight2, LOW);
  if (motorsStopped) {
    Serial.println("🚗 Turning Left");
    motorsStopped = false;
  }
}

void moveRight() {
  digitalWrite(enableA, HIGH); digitalWrite(enableB, HIGH);
  digitalWrite(motorLeft1, HIGH); digitalWrite(motorLeft2, LOW);
  digitalWrite(motorRight1, LOW); digitalWrite(motorRight2, HIGH);
  if (motorsStopped) {
    Serial.println("🚗 Turning Right");
    motorsStopped = false;
  }
}

void stopMotors() {
  if (!motorsStopped) {
    digitalWrite(enableA, LOW); digitalWrite(enableB, LOW);
    Serial.println("🛑 Motors Stopped");
    motorsStopped = true;
  }
}

// ---------------- Belt Functions ----------------
void stopBelts() {
  digitalWrite(enableBeltA, LOW); digitalWrite(enableBeltB, LOW);
}

void runRecyclableBelt() {
  stopMotors();
  processingWaste = true;
  Serial.println("♻️ Activating Recyclable Belt");
  
  digitalWrite(enableBeltA, HIGH);
  digitalWrite(beltLeft1, HIGH); digitalWrite(beltLeft2, LOW);
  delay(3000);
  stopBelts();
  processingWaste = false;
  moveForward();
}

void runNonRecyclableBelt() {
  stopMotors();
  processingWaste = true;
  Serial.println("🗑️ Activating Non-Recyclable Belt");
  
  digitalWrite(enableBeltB, HIGH);
  digitalWrite(beltRight1, HIGH); digitalWrite(beltRight2, LOW);
  delay(3000);
  stopBelts();
  processingWaste = false;
  moveForward();
}

// ---------------- Command Processing ----------------
void processCommand(String command) {
  command.trim();
  Serial.println("📩 Received: " + command);
  if (processingWaste) return;

  if (command == "FORWARD") moveForward();
  else if (command == "BACKWARD") moveBackward();
  else if (command == "LEFT") moveLeft();
  else if (command == "RIGHT") moveRight();
  else if (command == "STOP") stopMotors();
  else if (command == "Recyclable Belt") runRecyclableBelt();
  else if (command == "Non-Recyclable Belt") runNonRecyclableBelt();
}

// ---------------- WiFi Handling ----------------
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastWifiReconnectAttempt >= wifiReconnectInterval) {
      Serial.println("🔄 Attempting WiFi reconnection...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      lastWifiReconnectAttempt = currentMillis;
    }
  }
}

// ---------------- Setup ----------------
void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 Starting USV...");

  // Initialize Dabble (for manual mode)
  Dabble.begin("USV");
  Serial.println("✅ Dabble Initialized. Connect via Bluetooth");

  // Initialize Motor and Belt Pins
  pinMode(motorLeft1, OUTPUT); pinMode(motorLeft2, OUTPUT);
  pinMode(motorRight1, OUTPUT); pinMode(motorRight2, OUTPUT);
  pinMode(enableA, OUTPUT); pinMode(enableB, OUTPUT);
  pinMode(beltLeft1, OUTPUT); pinMode(beltLeft2, OUTPUT);
  pinMode(beltRight1, OUTPUT); pinMode(beltRight2, OUTPUT);
  pinMode(enableBeltA, OUTPUT); pinMode(enableBeltB, OUTPUT);

  stopMotors();
  stopBelts();

  // Connect to WiFi
  Serial.print("🔄 Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("✅ Server Started. Ready for commands!");
}

// ---------------- Main Loop ----------------
void loop() {
  Dabble.processInput();  // Process Dabble input for manual control
  checkWiFi();

  // Toggle Mode using Joystick Select (debounced)
  static unsigned long lastModeSwitch = 0;
  unsigned long currentMillis = millis();
  if (GamePad.isSelectPressed() && (currentMillis - lastModeSwitch > 500)) {
    lastModeSwitch = currentMillis;
    currentMode = (currentMode == AUTO_MODE) ? MANUAL_MODE : AUTO_MODE;
    Serial.print("🔄 Switched to ");
    Serial.println((currentMode == AUTO_MODE) ? "AUTO Mode" : "MANUAL Mode");
  }

  if (currentMode == AUTO_MODE) {
    // Auto Mode: Process WiFi commands
    WiFiClient client = server.available();
    if (client) {
      Serial.println("📡 Client Connected");
      String command = "";
      while (client.connected() && client.available()) {
        char c = client.read();
        if (c == '\n') {
          processCommand(command);
          break;
        } else if (c != '\r') {
          command += c;
        }
      }
      client.stop();
      Serial.println("🔌 Client Disconnected");
    }
  } else {
    // Manual Mode: Use Dabble GamePad controls
    if (GamePad.isUpPressed()) moveForward();
    else if (GamePad.isDownPressed()) moveBackward();
    else if (GamePad.isRightPressed()) moveRight();
    else if (GamePad.isLeftPressed()) moveLeft();
    else stopMotors();

    if (GamePad.isTrianglePressed()) runRecyclableBelt();
    if (GamePad.isCirclePressed()) runNonRecyclableBelt();
  }

  delay(10); // Prevent watchdog triggers
}
