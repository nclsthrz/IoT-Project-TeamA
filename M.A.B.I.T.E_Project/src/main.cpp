#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "pitches.h"

// ====================
// OLED SCREEN
// ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_ADDRESS  0x3C
#define I2C_SDA 18
#define I2C_SCL 19

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ====================
// PINS GPIO
// ====================
const uint8_t ledPins[] = {8, 7, 6, 5};
const uint8_t buttonPins[] = {0, 1, 2, 3};
#define SPEAKER_PIN 10
#define BTN_GREY 9
#define POT_PIN 4

// ====================
// SIMON GAME
// ====================
#define MAX_GAME_LENGTH 100
uint8_t gameSequence[MAX_GAME_LENGTH] = {0};
uint8_t gameIndex = 0;
uint8_t simonScore = 0;
const int gameTones[] = { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5 };

// ====================
// MENU
// ====================
int selectedOption = 1;
const int TOTAL_OPTIONS = 5;
bool inMenu = true;
unsigned long lastDebounceTime = 0;
const long debounceDelay = 50;
int lastPotValue = 0;

// ====================
// BEST SCORES IN-MEMORY (no EEPROM)
// These persist only until reboot.
// Simon: higher = better
// Timer: bestTimerMs = smallest deviation in ms (lower = better)
// Reflex: bestReflexMs = smallest average in ms (lower = better)
// Global: higher = better
// ====================
int bestSimon = 0;
int bestTimerMs = 0;
int bestReflexMs = 0;
int bestGlobal = 0;

// ====================
// FONCTIONS OLED
// ====================
void clearDisplay() {
  display.clearDisplay();
  display.display();
}

void showScore(int score) {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(40, 20);
  display.println(score);
  display.display();
}

// ====================
// SIMON GAME 
// ====================
void lightLedAndPlayTone(byte ledIndex) {
  digitalWrite(ledPins[ledIndex], HIGH);
  tone(SPEAKER_PIN, gameTones[ledIndex]);
  delay(300);
  digitalWrite(ledPins[ledIndex], LOW);
  noTone(SPEAKER_PIN);
  delay(50);
}

void playSequence() {
  for (int i = 0; i < gameIndex; i++) {
    lightLedAndPlayTone(gameSequence[i]);
  }
}

byte readButtons() {
  while (true) {
    for (byte i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        delay(50);
        while (digitalRead(buttonPins[i]) == LOW);
        return i;
      }
    }
    delay(1);
  }
}

bool checkUserSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte expected = gameSequence[i];
    byte actual = readButtons();
    lightLedAndPlayTone(actual);
    if (expected != actual) return false;
  }
  return true;
}

void playLevelUpSound() {
  tone(SPEAKER_PIN, NOTE_E4); delay(150);
  tone(SPEAKER_PIN, NOTE_G4); delay(150);
  tone(SPEAKER_PIN, NOTE_E5); delay(150);
  tone(SPEAKER_PIN, NOTE_C5); delay(150);
  tone(SPEAKER_PIN, NOTE_D5); delay(150);
  tone(SPEAKER_PIN, NOTE_G5); delay(150);
  noTone(SPEAKER_PIN);
}

int runSimonGame() {
  clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.println(F("SIMON"));
  display.display();
  delay(1500);

  simonScore = 1;
  gameIndex = 0;
  randomSeed(analogRead(POT_PIN));

  while (true) {
    showScore(simonScore);

    gameSequence[gameIndex] = random(0, 4);
    gameIndex++;
    if (gameIndex >= MAX_GAME_LENGTH) gameIndex = MAX_GAME_LENGTH - 1;

    playSequence();

    if (!checkUserSequence()) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setCursor(10, 10);
      display.println(F("GAME OVER"));
      display.setTextSize(3);
      display.setCursor(40, 35);
      display.println(simonScore);
      display.display();

      tone(SPEAKER_PIN, NOTE_DS5); delay(300);
      tone(SPEAKER_PIN, NOTE_D5);  delay(300);
      tone(SPEAKER_PIN, NOTE_CS5); delay(300);
      for (int i = 0; i < 10; i++) {
        for (int p = -10; p <= 10; p++) {
          tone(SPEAKER_PIN, NOTE_C5 + p); delay(6);
        }
      }
      noTone(SPEAKER_PIN);

      delay(2000);

      if (simonScore > bestSimon) bestSimon = simonScore;

      return simonScore;
    }

    simonScore++;
    playLevelUpSound();
    delay(500);
  }
}

// ====================
// 10 SEC TIMER GAME
// ====================
float run10SecTimer() {
  clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 20);
  display.println(F("READY ?"));
  display.display();
  delay(1500);

  clearDisplay();
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.println(F("GO! PRESS AT 10.000"));
  display.display();

  tone(SPEAKER_PIN, NOTE_C5, 200);
  delay(250);
  tone(SPEAKER_PIN, NOTE_E5, 200);
  delay(300);
  noTone(SPEAKER_PIN);

  unsigned long startTime = millis();
  bool playerPressed = false;

  while (!playerPressed) {
    unsigned long currentTime = millis() - startTime;
    if (currentTime > 12000) break;

    float elapsed = currentTime / 1000.0;

    display.clearDisplay();
    if (elapsed < 5.0) {
      display.setTextSize(2);
      display.setCursor(20, 15);
      display.printf("%.3f", elapsed);
      display.setTextSize(1);
      display.setCursor(95, 25);
      display.println(F("SEC"));
    }
    display.display();

    if (digitalRead(BTN_GREY) == LOW) {
      playerPressed = true;
      delay(50);
      while (digitalRead(BTN_GREY) == LOW);
    }
  }

  float playerTime = (millis() - startTime) / 1000.0;
  float deviation  = fabs(playerTime - 10.0) * 1000.0;

  if (deviation < 100) {
    clearDisplay();
    display.setTextSize(3);
    display.setCursor(25, 20);
    display.println(F("BRAVO"));
    display.display();

    int winMelody[]    = {NOTE_E5, NOTE_G5, NOTE_E6, NOTE_C6, NOTE_G5, NOTE_E5};
    int winDurations[] = {150, 150, 300, 300, 300, 500};
    for (int i = 0; i < 6; i++) {
      tone(SPEAKER_PIN, winMelody[i], winDurations[i]);
      delay(winDurations[i] * 1.3);
    }
    noTone(SPEAKER_PIN);
  } else {
    clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 5);
    display.println(F("GAP:"));
    display.setTextSize(3);
    display.setCursor(15, 30);
    display.printf("%.0fms", deviation);
    display.display();

    tone(SPEAKER_PIN, NOTE_G3, 200);
    delay(250);
    tone(SPEAKER_PIN, NOTE_DS3, 200);
    delay(300);
    noTone(SPEAKER_PIN);
  }

  delay(3000);

  int deviationMs = (int)round(deviation);
  if (deviationMs > 0 && (bestTimerMs == 0 || deviationMs < bestTimerMs)) {
    bestTimerMs = deviationMs;
  }

  return deviation;
}

// ====================
// REFLEX CHALLENGE GAME
// ====================
float runReflexChallenge() {
  clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.println(F("REFLEX"));
  display.println(F("CHALLENGE"));
  display.display();
  delay(2000);

  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  float reactionTimes[10];
  int currentTrial = 0;

  while (currentTrial < 10) {
    byte randomLed = random(0, 4);
    
    digitalWrite(ledPins[randomLed], HIGH);
    tone(SPEAKER_PIN, gameTones[randomLed], 100);
    unsigned long startTime = millis();
    
    byte correctButton = randomLed;
    while (true) {
      for (byte i = 0; i < 4; i++) {
        if (digitalRead(buttonPins[i]) == LOW) {
          delay(30);
          while (digitalRead(buttonPins[i]) == LOW);
          
          unsigned long reactionTime = millis() - startTime;
          float reactionSec = reactionTime / 1000.0;
          
          digitalWrite(ledPins[randomLed], LOW);
          noTone(SPEAKER_PIN);
          
          if (i == correctButton) {
            reactionTimes[currentTrial] = reactionSec;
            
            clearDisplay();
            display.setTextSize(2);
            display.setCursor(20, 15);
            display.printf("%.3fs", reactionSec);
            display.setTextSize(1);
            display.setCursor(10, 45);
            display.printf("Try %d/10", currentTrial + 1);
            display.display();
            
            tone(SPEAKER_PIN, NOTE_C5, 100);
            delay(150);
            noTone(SPEAKER_PIN);
            
            currentTrial++;
          } else {
            clearDisplay();
            display.setTextSize(2);
            display.setCursor(25, 25);
            display.println(F("ERROR!"));
            display.display();
            
            tone(SPEAKER_PIN, NOTE_G3, 300);
            delay(500);
            noTone(SPEAKER_PIN);
            digitalWrite(ledPins[randomLed], LOW);
          }
          delay(1000);
          goto nextReflexTrial;
        }
      }
    }
    nextReflexTrial:;
  }

  clearDisplay();
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.println(F("10th Try:"));
  display.setTextSize(3);
  display.setCursor(15, 35);
  display.printf("%.3fs", reactionTimes[9]);
  display.display();
  delay(1500);

  float total = 0;
  for (int i = 0; i < 10; i++) {
    total += reactionTimes[i];
  }
  float average = total / 10.0;

  clearDisplay();
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.println(F("MEAN:"));
  display.setTextSize(3);
  display.setCursor(15, 35);
  display.printf("%.3fs", average);
  display.display();

  tone(SPEAKER_PIN, NOTE_E5, 200);
  delay(250);
  tone(SPEAKER_PIN, NOTE_G5, 200);
  delay(250);
  tone(SPEAKER_PIN, NOTE_C6, 400);
  noTone(SPEAKER_PIN);

  delay(3000);

  int averageMs = (int)round(average * 1000.0);

  if (averageMs > 0 && (bestReflexMs == 0 || averageMs < bestReflexMs)) {
    bestReflexMs = averageMs;
  }

  return average;
}

void displayMenu();

// ====================
// GLOBAL COGNITIVE CHALLENGE
// ====================
void runGlobalCognitive() {
  inMenu = false;
  
  clearDisplay();
  display.setTextSize(2);
  display.setCursor(5, 10);
  display.println(F("GLOBAL"));
  display.println(F("COGNITIVE"));
  display.display();
  delay(2000);

  // 1. SIMON
  int simonFinal = runSimonGame();
  delay(1000);

  // 2. 10SEC
  float timerDeviation = run10SecTimer();
  delay(1000);

  // 3. REFLEX
  float reflexAverage = runReflexChallenge();
  delay(1000);

  // === GLOBAL SCORE ===
  int globalScore = (simonFinal * 10) + 
                    (int)( (timerDeviation>0 ? 1000.0 / timerDeviation : 0) ) + 
                    (int)( (reflexAverage>0 ? 1000.0 / (reflexAverage * 10) : 0) );

  clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 5);
  display.println(F("GLOBAL SCORE:"));
  display.setTextSize(3);
  display.setCursor(20, 30);
  display.println(globalScore);
  display.display();

  for (int i = 0; i < 3; i++) {
    tone(SPEAKER_PIN, NOTE_C6, 200);
    delay(250);
    tone(SPEAKER_PIN, NOTE_E6, 200);
    delay(250);
    tone(SPEAKER_PIN, NOTE_G6, 400);
    delay(500);
  }
  noTone(SPEAKER_PIN);

  if (globalScore > bestGlobal) bestGlobal = globalScore;

  delay(5000);
  inMenu = true;
  displayMenu();
}

// ====================
// SCOREBOARD
// ====================
void showScoreBoard() {
  clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.print(F("Simon: "));
  if (bestSimon > 0) display.println(bestSimon); else display.println(F("-"));

  display.setCursor(0, 15);
  display.print(F("Timer: "));
  if (bestTimerMs > 0) {
    display.print(bestTimerMs);
    display.println(F(" ms"));
  } else {
    display.println(F("- ms"));
  }

  display.setCursor(0, 30);
  display.print(F("Reflex: "));
  if (bestReflexMs > 0) {
    display.print(bestReflexMs / 1000.0, 3);
    display.println(F(" s"));
  } else {
    display.println(F("- s"));
  }

  display.setCursor(0, 45);
  display.print(F("Global: "));
  if (bestGlobal > 0) display.println(bestGlobal); else display.println(F("-"));

  display.display();

  tone(SPEAKER_PIN, NOTE_A4, 200);
  delay(250);
  noTone(SPEAKER_PIN);
}


// ====================
// MENU
// ====================
void displayMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 2);
  display.println(F("[WOWKI]"));

  const char* menuItems[] = {
    "1. LED GAME",
    "2. 10 SEC TIMER",
    "3. REFLEX CHALLENGE",
    "4. GLOBAL COGNITIVE",
    "5. SCORE BOARD"
  };

  int y = 15;
  for (int i = 0; i < TOTAL_OPTIONS; i++) {
    display.setCursor(10, y + i * 10);
    if (i + 1 == selectedOption) {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.fillRect(5, y + i * 10 - 1, 118, 10, SSD1306_WHITE);
    } else {
      display.setTextColor(SSD1306_WHITE);
    }
    display.println(menuItems[i]);
  }
  display.display();
}

void launchSelected() {
  inMenu = false;
  switch (selectedOption) {
    case 1: 
      runSimonGame(); 
      delay(1000);
      inMenu = true; 
      displayMenu(); 
      break;
    case 2: 
      run10SecTimer(); 
      delay(1000);
      inMenu = true; 
      displayMenu(); 
      break;
    case 3: 
      runReflexChallenge(); 
      delay(1000);
      inMenu = true; 
      displayMenu(); 
      break;
    case 4: runGlobalCognitive(); break;
    case 5: 
      showScoreBoard();
      delay(5000);
      inMenu = true; 
      displayMenu(); 
      break;
  }
}

// ====================
// SETUP & LOOP
// ====================
void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F("OLED failed!"));
    for(;;);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(BTN_GREY, INPUT_PULLUP);

  randomSeed(analogRead(POT_PIN));
  displayMenu();
}

void loop() {
  if (!inMenu) return;

  int potValue = analogRead(POT_PIN);
  int newOption = map(potValue, 0, 4095, 1, TOTAL_OPTIONS);
  newOption = constrain(newOption, 1, TOTAL_OPTIONS);

  if (abs(newOption - selectedOption) >= 1 || abs(potValue - lastPotValue) > 200) {
    selectedOption = newOption;
    lastPotValue = potValue;
    displayMenu();
    delay(50);
  }

  if (digitalRead(BTN_GREY) == LOW && (millis() - lastDebounceTime > debounceDelay)) {
    lastDebounceTime = millis();
    launchSelected();
  }
}
