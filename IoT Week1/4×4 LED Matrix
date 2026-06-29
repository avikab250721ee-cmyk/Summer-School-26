// 4x4 LED Matrix — Display Characters
// Row = anode (+), Column = cathode (-)

int rowPins[4] = {2, 3, 4, 5};    // ROW 1 to 4
int colPins[4] = {6, 7, 8, 9};    // COL 1 to 4

// =====================
// CHARACTER DEFINITIONS
// 1 = LED ON, 0 = LED OFF
// Each character = 4 rows x 4 cols
// =====================

// Letter C
byte letterC[4][4] = {
  {1, 1, 1, 1},
  {1, 0, 0, 0},
  {1, 0, 0, 0},
  {1, 1, 1, 1}
};

// Letter A
byte letterA[4][4] = {
  {0, 1, 1, 0},
  {1, 0, 0, 1},
  {1, 1, 1, 1},
  {1, 0, 0, 1}
};

// Letter R (example — replace with your initial)
byte letterR[4][4] = {
  {1, 1, 1, 0},
  {1, 0, 0, 1},
  {1, 1, 1, 0},
  {1, 0, 0, 1}
};

// Smiley face — just for fun!
byte smiley[4][4] = {
  {0, 1, 1, 0},
  {0, 0, 0, 0},
  {1, 0, 0, 1},
  {0, 1, 1, 0}
};

// All LEDs ON
byte allOn[4][4] = {
  {1, 1, 1, 1},
  {1, 1, 1, 1},
  {1, 1, 1, 1},
  {1, 1, 1, 1}
};

// =====================
// DISPLAY FUNCTION
// =====================
void displayPattern(byte pattern[4][4], int duration) {
  long startTime = millis();

  while (millis() - startTime < duration) {
    for (int row = 0; row < 4; row++) {

      // Activate this row (set HIGH)
      for (int r = 0; r < 4; r++) {
        digitalWrite(rowPins[r], LOW);   // turn off all rows first
      }
      digitalWrite(rowPins[row], HIGH);  // activate current row

      // Set columns based on pattern
      for (int col = 0; col < 4; col++) {
        if (pattern[row][col] == 1) {
          digitalWrite(colPins[col], LOW);   // LOW = cathode on = LED lights
        } else {
          digitalWrite(colPins[col], HIGH);  // HIGH = cathode off = LED dark
        }
      }

      delay(2); // hold each row for 2ms → full refresh in 8ms
    }
  }
}

// =====================
// SETUP
// =====================
void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(rowPins[i], OUTPUT);
    pinMode(colPins[i], OUTPUT);
    digitalWrite(rowPins[i], LOW);
    digitalWrite(colPins[i], HIGH);  // columns idle HIGH (cathode off)
  }
}

// =====================
// MAIN LOOP
// =====================
void loop() {
  displayPattern(letterC,  2000);  // show C for 2 seconds
  displayPattern(letterA,  2000);  // show A for 2 seconds
  displayPattern(letterR,  2000);  // show R for 2 seconds
  displayPattern(smiley,   1500);  // show smiley
  displayPattern(allOn,    1000);  // flash all on
}
