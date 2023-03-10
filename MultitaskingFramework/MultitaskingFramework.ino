#include "meta.h"
#include "main.h"

volatile unsigned long frameCtr = 0;     // Counts the number of frames that have elapsed since program start
volatile unsigned long ioTimestamp = 0;  // Store the time of the last pin change interrupt
volatile byte portBCapture = 0; // Track the states of the pins on port B
volatile byte portDCapture = 0; // Track the states of the pins on port D

// Accept an arbitrary number of IO pins as arguments and enable them in the pin change interrupt masks
template<typename... T>
void registerIOPins(const T... pinPack) {
  // If you mess up the arguments to registerIOPins, the default error message is really cryptic
  static_assert(all_true<(is_same<T, int>::value)...>::value, "All arguments to registerIOPins(...) must be integers");
  
  const byte numPins = sizeof...(T);
  const byte pins[numPins] = {pinPack...};
  for (byte i=0; i<numPins; i++) {
    if (pins[i] < 8) {
      PCMSK2 |= 1 << pins[i];
    }
    else if (pins[i] < 14) {
      PCMSK0 |= 1 << (pins[i] - 8);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);
  
  cli();  // Disable interrupts while changing interrupt registers

  // Setting up the draw loop
  WDTCSR = (1 << WDCE) | (1 << WDE);  // Enable watchdog timer
  WDTCSR = (1 << WDIE) | (1 << WDP1); // Enable watchdog interrupt and set it to happen at 16 Hz

  // Setting up the IO handler
  PCICR  |= 0b00000101; // Enable pin change interrupts for ports B and D
  PCMSK0 &= ~0x2F;      // Disable all pin change interrupts by default on port B
  PCMSK2 = 0;           // Disable all pin change interrupts by default on port D
  
  sei();  // Re-enable interrupts, including the newly configured ones
  
  int exitCode = Main();
  Serial.print("Main program finished with exit code ");
  Serial.println(exitCode);
}

void loop() {
  // Lookin kinda empty here
}

ISR(WDT_vect) {           // Watchdog timer interrupt handler
  Draw(frameCtr++);       // Call the draw function and update the frame counter
  if (Serial.available()) // Check if any serial data is available
    SerialEvent();        // Call the serial event handler if so
}

ISR(PCINT0_vect) {
  byte portBState = PINB;                                   // Get the state of the pins of port B
  IO(micros() - ioTimestamp, portBState ^ portBCapture, 0); // Call the IO function, passing in whatever changed on port B
  ioTimestamp = micros();
  portBCapture = portBState;                                // Update the capture variable to hold the new state
}

ISR(PCINT2_vect) {
  byte portDState = PIND;                                   // Get the state of the pins of port D
  IO(micros() - ioTimestamp, 0, portDState ^ portDCapture); // Call the IO function, passing in whatever changed on port D
  ioTimestamp = micros();
  portDCapture = portDState;                                // Update the capture variable to hold the new state
}
