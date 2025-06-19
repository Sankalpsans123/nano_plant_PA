#include <DHT.h>

// === PIN DEFINITIONS ===
#define DHTPIN 13
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int segPins[] = {11, 7, 4, 2, A2, 10, 5};
const int dpPin = 3;
const int digitPins[] = {12, 9, 8, 6};

const int buttonPin = A0;
const int soilPin = A1;

// === DISPLAY & TIME ===
const byte digitEncoding[10] = {
  B0000001, B1001111, B0010010, B0000110,
  B1001100, B0100100, B0100000, B0001111,
  B0000000, B0000100
};

int hours = 12, minutes = 0;
unsigned long lastMinuteUpdate = 0;
unsigned long lastColonBlink = 0;
unsigned long lastDigitSwitch = 0;
bool colonOn = false;
int currentDigit = 0;

// === BUTTON/SENSOR MODE ===
bool buttonPressed = false;
unsigned long buttonPressTime = 0;
const unsigned long showSensorDuration = 5000;
float lastTemp = 0, lastHumid = 0;

enum Mode { CLOCK, SENSOR };
Mode currentMode = CLOCK;

// === SERIAL CHAT MODE ===
bool chatMode = false;
unsigned long lastSoilReminder = 0;
const unsigned long reminderInterval = 300000; // 5 mins

void setup() {
  for (int i = 0; i < 7; i++) pinMode(segPins[i], OUTPUT);
  for (int i = 0; i < 4; i++) pinMode(digitPins[i], OUTPUT);
  pinMode(dpPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(soilPin, INPUT);

  dht.begin();
  Serial.begin(9600);
  Serial.println("🌿 Smart Plant Booted. Use 'SET HH MM' or 'start_chat' to begin talking.");
}

void loop() {
  unsigned long now = millis();

  checkSerialCommand();

  // Update time
  if (now - lastMinuteUpdate >= 60000) {
    lastMinuteUpdate = now;
    minutes++;
    if (minutes >= 60) { minutes = 0; hours++; }
    if (hours >= 24) hours = 0;
  }

  // Blink colon
  if (now - lastColonBlink >= 1000) {
    colonOn = !colonOn;
    lastColonBlink = now;
  }

  // Show sensor on button press
  if (digitalRead(buttonPin) == LOW && !buttonPressed) {
    delay(50);
    if (digitalRead(buttonPin) == LOW) {
      buttonPressed = true;
      buttonPressTime = now;
      currentMode = SENSOR;

      lastTemp = dht.readTemperature();
      lastHumid = dht.readHumidity();
    }
  }

  if (currentMode == SENSOR && now - buttonPressTime > showSensorDuration) {
    currentMode = CLOCK;
    buttonPressed = false;
  }

  // Display
  if (now - lastDigitSwitch >= 2) {
    lastDigitSwitch = now;
    if (currentMode == CLOCK)
      displayTime(currentDigit);
    else
      displaySensor(currentDigit);
    currentDigit = (currentDigit + 1) % 4;
  }

  // Soil reminder
  if (!chatMode && now - lastSoilReminder >= reminderInterval) {
    lastSoilReminder = now;
    if (isSoilDry())
      Serial.println("💧 I'm thirsty! Give me water!");
  }
}

// === DISPLAY ===
void displayTime(int index) {
  int digits[4] = {
    hours / 10, hours % 10,
    minutes / 10, minutes % 10
  };
  showDigit(index, digits[index], index == 1 && colonOn);
}

void displaySensor(int index) {
  int value = (index < 2) ? (int)lastTemp : (int)lastHumid;
  int digit = (index % 2 == 0) ? value / 10 : value % 10;
  showDigit(index, digit, false);
}

void showDigit(int index, int num, bool dp) {
  for (int i = 0; i < 4; i++) digitalWrite(digitPins[i], LOW);
  for (int i = 0; i < 7; i++) digitalWrite(segPins[i], HIGH);
  digitalWrite(dpPin, HIGH);

  byte pattern = digitEncoding[num];
  for (int i = 0; i < 7; i++)
    digitalWrite(segPins[i], bitRead(pattern, 6 - i));

  if (dp) digitalWrite(dpPin, LOW);
  digitalWrite(digitPins[index], HIGH);
}

// === CHATBOT COMMAND HANDLER ===
void checkSerialCommand() {
  static String inputBuffer = "";

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      inputBuffer.trim();
      inputBuffer.toLowerCase();

      if (inputBuffer.startsWith("set ")) {
        int sp1 = inputBuffer.indexOf(' ');
        int sp2 = inputBuffer.indexOf(' ', sp1 + 1);
        if (sp1 != -1 && sp2 != -1) {
          int h = inputBuffer.substring(sp1 + 1, sp2).toInt();
          int m = inputBuffer.substring(sp2 + 1).toInt();
          if (h >= 0 && h < 24 && m >= 0 && m < 60) {
            hours = h;
            minutes = m;
            Serial.print("🕒 Time set to ");
            Serial.print(hours); Serial.print(":");
            if (minutes < 10) Serial.print("0");
            Serial.println(minutes);
          } else {
            Serial.println("❌ Invalid format. Try: SET HH MM");
          }
        } else Serial.println("❌ Try: SET HH MM");
      }

      else if (inputBuffer == "start_chat") {
        chatMode = true;
        Serial.println("🤖 Chat mode ON. Type something!");
      }
      else if (inputBuffer == "exit_chat") {
        chatMode = false;
        Serial.println("👋 Chat mode OFF.");
      }

      else if (chatMode) {
        if ((inputBuffer.startsWith("how") || inputBuffer.startsWith("hello") || inputBuffer.startsWith("hey")) &&
             inputBuffer.indexOf("you") != -1) {
          String replies[] = {"Hey friend! 🌿", "I'm feeling fresh and photosynthesizing!", "All green and good!"};
          Serial.println(replies[random(3)]);
        }
        else if (inputBuffer.indexOf("do") != -1 && inputBuffer.indexOf("want") != -1 && inputBuffer.indexOf("water") != -1) {
          if (isSoilDry()) {
            String thirsty[] = {"💧 Yes please! I'm as dry as the Sahara!", "🥵 Water me before I wilt!", "😢 Parched and waiting!"};
            Serial.println(thirsty[random(3)]);
          } else {
            String full[] = {"😎 Nah, I'm hydrated!", "💦 I just had my drink!", "🌊 Thanks, but I'm full."};
            Serial.println(full[random(3)]);
          }
        }
        else if (inputBuffer.indexOf("what") != -1 && inputBuffer.indexOf("time") != -1) {
          Serial.print("🕐 It's ");
          Serial.print(hours); Serial.print(":");
          if (minutes < 10) Serial.print("0");
          Serial.println(minutes);
        }
        else if (inputBuffer.indexOf("what") != -1 &&
                (inputBuffer.indexOf("temperature") != -1 || inputBuffer.indexOf("temp") != -1)) {
          float t = dht.readTemperature();
          float h = dht.readHumidity();
          Serial.print("🌡 It's "); Serial.print(t); Serial.print("°C with ");
          Serial.print(h); Serial.println("% humidity.");
        }
        else if (inputBuffer.length() > 20) {
          String affirm[] = {"Nice one!", "Cool story bro 😎", "That's wild!", "Great stuff!"};
          Serial.println(affirm[random(4)]);
        }
        else {
          Serial.println("🤔 I didn't catch that...");
        }
      }

      else {
        Serial.println("🤖 Unknown command. Try: SET HH MM or start_chat");
      }

      inputBuffer = "";
    } else {
      inputBuffer += c;
    }
  }
}

bool isSoilDry() {
  return digitalRead(soilPin) == HIGH;  // HIGH = dry
}
