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

// Set the framerate to a power of two. Range: -3 - 6 (8s/frame - 64Hz)
void setFramerate(int8_t exp2) {
  if (-3 <= exp2 && exp2 <= 6) {
    byte newPrescaler = 0x6 - exp2;                               // Calculate the new value to be assigned to WDP
    if (newPrescaler > 7) newPrescaler = 0x20 + (newPrescaler&7); // The WDP3 bit is not adjacent to WDP0-2; correct for this if necessary
    WDTCSR = (WDTCSR & 0xE8) | newPrescaler;                      // Update the watchdog prescaler with only one write
  }
}

void setup() {
  Serial.begin(115200);
  delay(50);

  pinMode(3, OUTPUT);
  
  cli();  // Disable interrupts while changing interrupt registers

  // Setting up the draw loop
  WDTCSR = bit(WDCE) | bit(WDE);  // Enable watchdog timer
  WDTCSR = bit(WDIE) | bit(WDP1); // Enable watchdog interrupt and set it to happen at 16 Hz

  // Setting up the IO handler
  PCICR  |= 0b00000101; // Enable pin change interrupts for ports B and D
  PCMSK0 &= ~0x2F;      // Disable all pin change interrupts by default on port B
  PCMSK2 = 0;           // Disable all pin change interrupts by default on port D

  // Setting up the audio handler
    // Timer 2 - fast PWM for speaker
  OCR2B = 255;
  TCCR2A = bit(COM2B1) | bit(COM2B0) | bit(WGM21) | bit(WGM20); // Fast PWM mode, OC2B (pin 3) compare match interrupt enabled
  TCCR2B = bit(CS20);                                           // No prescaler (~31kHz PWM freq.)
    // Timer 0 - already running, overflow vector used by millis()
  OCR0A = 100;            // Set timer 0 output compare register A to something high (will trigger an interrupt when TCNT0 matches OCR0A)
  TCCR0A |= bit(WGM00) | bit(WGM01);
  TCCR0B &= ~bit(WGM02);
  TIMSK0 = bit(TOIE0) | bit(OCIE0A);  // Enable compare A match interrupt
  
  sei();  // Re-enable interrupts, including the newly configured ones
  
  int exitCode = Main();
  Serial.print("Main program finished with exit code ");
  Serial.println(exitCode);
}

void loop() {
  // Lookin kinda empty here
}

volatile bool stackSmashing = false;
ISR(WDT_vect, ISR_NOBLOCK) {  // Watchdog timer interrupt handler - the Draw() function could take a long time, so allow this interrupt to be interrupted by other interrupts
  if (stackSmashing)          // This ISR interrupted itself; we need to finish the previous interrupt call or we run the risk of a stack overflow from repeatedly nesting calls
    return;                   // Immediately return from the *nested* call - this will put us back to work on the previous call and give it time to finish
  stackSmashing = true;       // We aren't done until this is set back to false
  Draw(frameCtr++);           // Call the draw function and update the frame counter
  if (Serial.available())     // Check if any serial data is available
    SerialEvent();            // Call the serial event handler if so
  stackSmashing = false;
}

volatile unsigned long t = 0;
ISR(TIMER0_COMPA_vect, __attribute__((ISR_FLATTEN))) {  // Audio handler - fires when TCNT0 matches OCR0A; the compiler will inline all function calls within this ISR
  TCCR0A &= ~(bit(WGM00) | bit(WGM01)); // In Fast PWM mode, OCRx registers are not updated until the next timer overflow; briefly put the timer into normal mode so we can update OCR0A immediately
  OCR0A += 31;                          // Timer counts to 256 every 1.024ms, so schedule the interrupt to happen again roughly 125us from now (~8kHz), accounting for ISR call overhead
  TCCR0A |= bit(WGM00) | bit(WGM01);    // Put the timer back into Fast PWM mode
  OCR2B = Audio(t++);                   // Generate an audio sample and increment t
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
