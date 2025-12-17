# IoT_TeamA
This is the github repository for the IOT project

# M.A.B.I.T.E  
**Mental Ability Benchmarking and Intelligence Testing Environment**

## 1. Project Context

M.A.B.I.T.E (Mental Ability Benchmarking and Intelligence Testing Environment) is an embedded systems project developed in an academic context.  
The objective of this project is to design an interactive cognitive assessment platform using an ESP32 microcontroller.

The system evaluates and trains different mental abilities such as:
- Short-term memory
- Reaction speed
- Time perception

The project combines hardware interaction, real-time user input, visual feedback, and audio signals, all simulated using the **Wokwi** environment.

---

## 2. System Architecture

### 2.1 Hardware Components

- ESP32-C3 DevKit microcontroller
- SSD1306 OLED display (128×64, I2C)
- 4 LEDs (Red, Green, Blue, Yellow)
- 4 colored push buttons (matching LEDs)
- 1 gray push button (menu validation)
- 1 potentiometer (menu navigation)
- 1 piezo buzzer

### 2.2 User Interface

- **Potentiometer**: navigates through the menu options  
- **Gray button**: validates the selected option  
- **Colored buttons**: used for gameplay interactions  
- **OLED display**: provides visual feedback and game information  
- **LEDs and buzzer**: provide real-time visual and audio cues  

---

## 3. Software Structure

The software is written in **C++ using the Arduino framework** and is organized into the following main modules:

- Menu management
- OLED display handling
- Individual mini-games
- Score management
- Global cognitive challenge

The program uses a state-based approach, switching between menu navigation and gameplay modes.

---

## 4. Menu Navigation Logic

The menu contains five options:


    LED GAME (Simon Says)

    10 Second Timer

    Reflex Challenge

    Global Cognitive Challenge

    Score Board

Menu selection is controlled by mapping the potentiometer’s analog value to a discrete menu index.  
The gray button confirms the selection.

---

## 5. Reflex Challenge Game  
### *Implemented by me*

### 5.1 Educational Objective

The Reflex Challenge game is designed to evaluate and improve:
- Reaction speed
- Attention
- Sensorimotor coordination

The objective is to measure how fast a player can react to a visual stimulus.

---

### 5.2 Functional Description

1. The system waits for a random delay.
2. A random LED is activated.
3. The reaction timer starts simultaneously.
4. The player must press the corresponding button as fast as possible.
5. The reaction time is measured in milliseconds.
6. The result is displayed on the OLED screen.
7. The game repeats for multiple attempts.
8. An average reaction time is calculated and displayed at the end.

---

### 5.3 Reflex Challenge – Functional Flow Diagram

Start Reflex Challenge game  
→ Initialize game variables (attempt counter reset, timers cleared)  
→ Generate random delay  
→ Turn on random LED  
→ Start reaction timer  
→ Wait for player input  

→ Compare player input with expected LED  
 → If the input is correct  
  → Stop timer  
  → Store reaction time  
  → Display reaction time  

 → If the input is incorrect  
  → Apply penalty  
  → Store penalty time  

→ Repeat until all attempts are completed  
→ Calculate average reaction time  
→ Display final score  
→ Update best score if applicable  

→ Return to menu

### 5.4 Key Functions Explanation

#### `runReflexChallenge()`
Main control loop of the Reflex Challenge game:
- Initializes timers and variables
- Manages game flow
- Displays reaction times
- Handles penalties
- Updates best score

#### `activateRandomLed()`
Selects and activates a random LED to generate the visual stimulus.

#### `readButtons()`
Continuously checks the state of the colored buttons and returns the index of the pressed button.

#### `measureReactionTime()`
Measures the time between LED activation and button press.

#### `updateBestScore()`
Updates the best reaction time score if the current performance is better.

---

## 6. Other Mini-Games Overview

### 6.1 10-Second Timer Challenge
Tests the player’s internal time estimation by asking them to press a button exactly after 10 seconds.

### 6.2 Simon Says Game
Tests short-term memory by asking the player to reproduce increasingly long sequences of colors.

---

## 7. Global Cognitive Challenge

This mode sequentially executes:
1. Simon Says
2. 10-Second Timer
3. Reflex Challenge

A global score is calculated using the results of all three games, providing an overall cognitive performance indicator.

---

## 8. Score Management

Best scores are stored temporarily in RAM:
- Best Simon score
- Best timer deviation (ms)
- Best reflex average (ms)
- Best global score

Scores reset when the system is powered off.

---

## 9. Simulation Environment

The entire system is simulated using **Wokwi**, allowing:
- Hardware validation
- Rapid testing
- Debugging without physical components

---

## 10. Conclusion

M.A.B.I.T.E demonstrates how embedded systems can be used to create interactive cognitive evaluation tools.  
The Reflex Challenge game highlights the integration of hardware control, user interaction, and precise time measurement in a real-time system.

---

## Author

Developed as part of an academic IOT project.  
Reflex Challenge game implementation by **Nicolas**.
