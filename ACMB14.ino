#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// === LCD and Keypad ===
LiquidCrystal_I2C lcd(0x27, 16, 2);
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {47, 46, 45, 44};
byte colPins[COLS] = {43, 42, 41, 40};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// === Outputs ===
// Valve 1 motor (L298)
int VALVE1_IN1 = 9;
int VALVE1_IN2 = 31;
int VALVE1_EN = 8;

// Valve 2 motor (L298)
int VALVE2_IN1 = 25;
int VALVE2_IN2 = 26;
int VALVE2_EN = 27;

// Common System Outputs
int MIXER_RELAY_PIN = 32;
int VALVE_MOTOR_IN1 = 33;
int VALVE_MOTOR_IN2 = 34;
int VALVE_MOTOR_EN = 28;

int FEED_MOTOR_IN = 35;
int FEED_MOTOR_EN = 29;

int EXTRUDER_RELAY_PIN = 36;
int MIXER_SOLENOID_VALVE_PIN = 39;

int BTN_OPEN = 2;    // Button to stop valve opening
int BTN_CLOSE = 38;   // Button to stop valve closing

// Valve open times (in milliseconds)
int valve1Time = 0;
int valve2Time = 0;

// === Flags ===
bool systemStarted = false;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  // Set pin modes for all outputs
  pinMode(VALVE1_IN1, OUTPUT);
  pinMode(VALVE1_IN2, OUTPUT);
  pinMode(VALVE1_EN, OUTPUT);

  pinMode(VALVE2_IN1, OUTPUT);
  pinMode(VALVE2_IN2, OUTPUT);
  pinMode(VALVE2_EN, OUTPUT);

  pinMode(MIXER_RELAY_PIN, OUTPUT);
  pinMode(VALVE_MOTOR_IN1, OUTPUT);
  pinMode(VALVE_MOTOR_IN2, OUTPUT);
  pinMode(VALVE_MOTOR_EN, OUTPUT);

  pinMode(FEED_MOTOR_IN, OUTPUT);
  pinMode(FEED_MOTOR_EN, OUTPUT);

  pinMode(EXTRUDER_RELAY_PIN, OUTPUT);
  pinMode(MIXER_SOLENOID_VALVE_PIN, OUTPUT);

  pinMode(BTN_OPEN, INPUT_PULLUP);
  pinMode(BTN_CLOSE, INPUT_PULLUP);
  

  // Set all motors OFF
  digitalWrite(VALVE1_IN1, HIGH
  );
  digitalWrite(VALVE2_IN1, HIGH);
  digitalWrite(VALVE_MOTOR_IN1, LOW);
  digitalWrite(FEED_MOTOR_IN, HIGH);
  digitalWrite(MIXER_RELAY_PIN, HIGH);
  digitalWrite(EXTRUDER_RELAY_PIN, HIGH);
  digitalWrite(MIXER_SOLENOID_VALVE_PIN, HIGH);

  // Prompt user to select ratio and start
  selectFixedRatio();
  promptStart();
}

void loop() {
  if (systemStarted) {
    feedMaterials();
    mixMaterials();
    //openAndCloseValve();
    feedToExtruder();

    // Reset flag for next cycle
    systemStarted = false;
    promptStart(); // Ask again to restart
  }
}

void selectFixedRatio() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Select Ratio:");
  lcd.setCursor(0, 1);
  lcd.print("A:4:1 B:1:1 C:2:1");

  char key = 0;
  while (true) {
    key = keypad.getKey();
    if (key == 'A') {
      valve1Time = 10000;
      valve2Time = 2500;
      break;
    } else if (key == 'B') {
      valve1Time = 5000;
      valve2Time = 5000;
      break;
    } else if (key == 'C') {
      valve1Time = 6000;
      valve2Time = 3000;
      break;
    }
  }

  lcd.clear();
  lcd.print("Ratio selected");
  delay(1000);
}

void promptStart() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press D to Start");

  while (true) {
    char key = keypad.getKey();
    if (key == 'D') {
      systemStarted = true;
      break;
    }
  }

  lcd.clear();
  lcd.print("Starting...");
  delay(1000);
}

void feedMaterials() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Feeding Mat 1...");

  digitalWrite(VALVE1_IN1, LOW);
  digitalWrite(VALVE1_IN2, LOW);
  digitalWrite(VALVE1_EN, HIGH);
  delay(valve1Time);
  digitalWrite(VALVE1_IN1, HIGH);

  lcd.setCursor(0, 1);
  lcd.print("Feeding Mat 2...");

  digitalWrite(VALVE2_IN1, LOW);
  digitalWrite(VALVE2_IN2, LOW);
  digitalWrite(VALVE2_EN, HIGH);
  delay(valve2Time);
  digitalWrite(VALVE2_IN1, HIGH);

  delay(1000);
}

void mixMaterials() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Filling mixer...");
  digitalWrite(MIXER_SOLENOID_VALVE_PIN, LOW);
  delay(5000);
  digitalWrite(MIXER_SOLENOID_VALVE_PIN, HIGH);

  lcd.setCursor(0, 1);
  lcd.print("Mixing...");
  digitalWrite(MIXER_RELAY_PIN, LOW);
  delay(6000); // 1 minute mixing
  digitalWrite(MIXER_RELAY_PIN, HIGH);
}

void openAndCloseValve() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Opening Valve...");

  // Open valve (rotate in one direction)
  digitalWrite(VALVE_MOTOR_IN1, HIGH);
  digitalWrite(VALVE_MOTOR_IN2, LOW);
  digitalWrite(VALVE_MOTOR_EN, HIGH);

  // Wait for OPEN button
  while (analogRead(BTN_OPEN) < 122 ); // Wait until pressed
  digitalWrite(VALVE_MOTOR_EN, LOW);     // Stop motor
  lcd.setCursor(0, 1);
  lcd.print("Open Stopped");

  delay(5000); // Pause before closing

  lcd.clear();
  lcd.print("Closing Valve...");

  // Close valve (reverse direction)
  digitalWrite(VALVE_MOTOR_IN1, LOW);
  digitalWrite(VALVE_MOTOR_IN2, HIGH);
  digitalWrite(VALVE_MOTOR_EN, HIGH);

  // Wait for CLOSE button
  while (digitalRead(BTN_CLOSE) == HIGH); // Wait until pressed
  digitalWrite(VALVE_MOTOR_EN, LOW);
  lcd.setCursor(0, 1);
  lcd.print("Closed");
  delay(2000);
}

void feedToExtruder() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Feeding Extruder");

  digitalWrite(FEED_MOTOR_IN, LOW);
  digitalWrite(FEED_MOTOR_EN, LOW);
  delay(60000);
  digitalWrite(FEED_MOTOR_IN, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Extruding...");
  digitalWrite(EXTRUDER_RELAY_PIN, LOW );
  delay(3000);
  digitalWrite(EXTRUDER_RELAY_PIN, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cycle Done");
  delay(3000);
}
