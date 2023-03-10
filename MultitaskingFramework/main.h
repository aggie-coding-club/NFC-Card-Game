#include <Arduino.h>

#define PWM_PIN 11  // Connect an LED or something to visualize analog output

#define ENC_A 2     // Encoder CLK pin (A phase)
#define ENC_B 3     // Encoder DT pin  (B phase)
#define ENC_SW 4    // Encoder SW pin  (button)

template <typename... T>
extern void registerIOPins(const T...);

volatile int encoderCounts = 0;

// Put any blocking storyline code in here
int Main() {
  Serial.println(F("Main function running! Enter some text to have it echoed back to you."));
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  registerIOPins(ENC_A, ENC_B, ENC_SW);
  
  while (true) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

// Automagically gets called 16 times per second
inline void Draw(unsigned long frame) {
  analogWrite(PWM_PIN, (byte)encoderCounts + 50);  // Update the PWM-controlled LED's brightness according to the encoder rotation
}

// Automagically gets called any time an IO pin changes state
inline void IO(unsigned long deltaT, byte deltaB, byte deltaD) {
  if (deltaT < 1000) return;                                                      // Crappy debouncing
  if (digitalRead(ENC_SW) == LOW) Serial.println(F("*snoot booping detected*"));  // Encoder button got pressed
  if (deltaD & 0xC) {                                                             // Something happened on the encoder phase pins
    bool countDir = bool(deltaD & 0x8) ^ digitalRead(ENC_A) ^ digitalRead(ENC_B); // If this is 1, the encoder turned clockwise; 0, counterclockwise
    encoderCounts += countDir ? 1 : -1;                                           // Update the encoder count accordingly
  }
}

// Automagically gets called whenever serial data is available
inline void SerialEvent() {
  while(Serial.available()) Serial.print((char)Serial.read());  // Echo back the received text
}
