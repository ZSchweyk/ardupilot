#include <Adafruit_MCP23X17.h>

Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;

// Pin definitions
const int interruptPin = 31;

// Pins to read (ignoring 1,4,6,9,11,13,15)
// connector mapping = {2, 3, 5, 7, 8, 10, 12, 14, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};

volatile bool readFlag = false;
volatile uint32_t ledStates = 0;






uint8_t reformLED(uint32_t raw, int* order, int size) {
  uint8_t result = 0;
  for (int i = 0; i < size; i++) {
    result ^= (raw >> order[i]) & 1UL;
    result = result << 1;
  }
  result = result >> 1;
  return result;
}


float direction_angle(uint8_t direction_bits) {
  float pi = 3.1415926536;

  if (direction_bits == 0b00010000) return -pi / 2;
  if (direction_bits == 0b00011000) return -67.5 * (pi / 180);
  if (direction_bits == 0b00001000) return -pi / 4;
  if (direction_bits == 0b00001100) return -22.5 * (pi / 180);
  if (direction_bits == 0b00000100) return 0;  // up, 0 deg
  if (direction_bits == 0b00000110) return 22.5 * (pi / 180);
  if (direction_bits == 0b00000010) return pi / 4;
  if (direction_bits == 0b00000011) return 67.5 * (pi / 180);
  return pi / 2;
}


// Must read out GPIO pins in order of a = LSB, g = MSB
uint8_t getDigitFromMask(uint8_t mask, uint8_t left) {
    // Standard Common Cathode Lookup Table
    static const uint8_t segmentTableRight[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67
    };

    // static const uint8_t segmentTableLeft[] = {
    //     0x3F, 0x16, 0x5B, 0x5F, 0x76, 0x6D, 0x7D, 0x07, 0x7F, 0x67
    // };

    if (left) {
      for (uint8_t i = 0; i < 10; ++i) {
        if (segmentTableRight[i] == mask) {
            return i;
        }
      }
    } else {
      for (uint8_t i = 0; i < 10; ++i) {
        if (segmentTableRight[i] == mask) {
            return i;
        }
      }
    }

    

    // Return a sentinel value (255) if no match is found
    return 0xFF; 
}

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial Monitor to open
  Serial.println("MCP23xxx Input Monitor Starting...");

  if (!mcp1.begin_I2C(0x24)) {
    Serial.println("Error: Could not find first MCP23017 chip.");
    while (1);
  }

  if (!mcp2.begin_I2C(0x20)) {
    Serial.println("Error: Could not find second MCP23017 chip.");
    while (1);
  }

  for (int i = 0; i < 16; i++) {
    mcp1.pinMode(i, INPUT_PULLUP);
  }

  for (int i = 0; i < 16; i++) {
    mcp2.pinMode(i, INPUT_PULLUP);
  }
}

void loop() {
  if (mcp2.digitalRead(15) == 0) {
    ledStates = 0;

    // Read all relevant pins
    for (int i = 0; i < 16; i++) {
      if (mcp1.digitalRead(i) == 0) {
        ledStates |= (1UL << i); // store pin state as a bit
      }
    }

    for (int i = 16; i < 32; i++) {
      if (mcp2.digitalRead(i - 16) == 0) {
        ledStates |= (1UL << i); // store pin state as a bit
      }
    }
    Serial.println(ledStates, BIN);

    // Decode the pattern
    int leftDigitSegs[] = {1, 0, 28, 30, 29, 2, 13};
    int rightDigitSegs[] = {11, 3, 7, 8, 6, 5, 4};
    int directionSegs[] = {15, 14, 12, 10, 9};
    uint8_t reformed_left_digit = reformLED(ledStates, leftDigitSegs, 7);
    uint8_t reformed_right_digit = reformLED(ledStates, rightDigitSegs, 7);
    uint8_t mid = reformLED(ledStates, directionSegs, 5);
    float direction = direction_angle(mid);
    uint8_t left_digit = getDigitFromMask(reformed_left_digit, 1);
    uint8_t right_digit = getDigitFromMask(reformed_right_digit, 0);
    Serial.print(left_digit);
    Serial.print(" | ");
    Serial.print(right_digit);
    Serial.print(" | ");
    Serial.print(mid);
    Serial.print(" | ");
    Serial.println(direction);
    delay(500);
  }
}
