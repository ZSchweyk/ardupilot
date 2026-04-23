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
    result ^= (raw >> order[i]) & 1U;
    result = result << 1;
  }
  return result;
}

// Must read out GPIO pins in order of a = LSB, g = MSB
uint8_t getDigitFromMask(uint8_t mask) {
    // Standard Common Cathode Lookup Table
    static const uint8_t segmentTable[] = {
        0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x67
    };

    for (uint8_t i = 0; i < 10; ++i) {
        if (segmentTable[i] == mask) {
            return i;
        }
    }

    // Return a sentinel value (255) if no match is found
    return 0xFF; 
}

// Interrupt Service Routine
void IRAM_ATTR onInterrupt() {
  readFlag = true;
  Serial.println("hi");
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

  // Set pin modes
  pinMode(interruptPin, INPUT_PULLUP); // interrupt pin

  // Attach interrupt
  attachInterrupt(digitalPinToInterrupt(interruptPin), onInterrupt, FALLING);
}

void loop() {
  Serial.print("LOW pins: ");

  bool first = true;

  // mcp1 (0–15)
  for (int i = 0; i < 16; i++) {
    if (mcp1.digitalRead(i) == 0) {
      if (!first) Serial.print(", ");
      Serial.print(i);
      first = false;
    }
  }

  // mcp2 (16–31)
  for (int i = 16; i < 32; i++) {
    if (mcp2.digitalRead(i - 16) == 0) {
      if (!first) Serial.print(", ");
      Serial.print(i);
      first = false;
    }
  }

  Serial.println();
  delay(100);
}