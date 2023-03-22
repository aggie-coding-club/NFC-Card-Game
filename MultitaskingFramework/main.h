#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>

// Connect a speaker or buzzer on pin 3 for sound effects

#define PWM_PIN 5   // Connect an LED or something to visualize analog output

#define ENC_A 2     // Encoder CLK pin (A phase)
#define ENC_B 4     // Encoder DT pin  (B phase)
#define ENC_SW 8    // Encoder SW pin  (button)

#define SDA_PIN 10  // RFID module SDA pin
#define RST_PIN 9   // RFID module RST pin

template <typename... T>
extern void registerIOPins(const T...);
extern void setFramerate(int8_t);

volatile int encoderCounts = 0;

MFRC522 mfrc522(SDA_PIN, RST_PIN);
MFRC522::StatusCode status;

byte buffer[18];
byte size = sizeof(buffer);

uint8_t pageAddr = 0x06;

// Put any blocking storyline code in here
int Main() {
  Serial.println(F("Main function running! Enter some text to have it echoed back to you."));
  pinMode(PWM_PIN, OUTPUT);
  cli();
  OCR0B = 0;
  TCCR0A |= bit(COM0B1);
  sei();
  pinMode(ENC_A, INPUT_PULLUP);
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);
  registerIOPins(ENC_A, ENC_B, ENC_SW);

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW); // Cheeky button hack

  setFramerate(4);  // Call the Draw() handler 2 ** (4) = 16 times per second

  SPI.begin();        // Initialize the SPI bus
  mfrc522.PCD_Init(); // Initialize the MFRC522  
  memcpy(buffer,"Hello, World! :D",16);
  
  while (true) {
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent())
      continue;
  
    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial())
      continue;
  
    // Write data ***********************************************
    for (int i=0; i < 4; i++) {
      // Data is writen in blocks of 4 bytes (4 bytes per page)
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Ultralight_Write(pageAddr+i, &buffer[i*4], 4);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Ultralight_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        continue;
      }
    }
    Serial.println(F("MIFARE_Ultralight_Write() OK "));
    Serial.println();
  
  
    // Read data ***************************************************
    Serial.println(F("Reading data ... "));
    // Data in the 4 blocks is all read at once
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(pageAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Read() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      continue;
    }
  
    Serial.print(F("Read data: "));
    //Dump a byte array to Serial
    for (byte i = 0; i < 16; i++) {
      Serial.write(buffer[i]);
    }
    Serial.println();
  
    mfrc522.PICC_HaltA();
  }
}

// Automagically gets called 16 times per second
inline void Draw(unsigned long frame) {
  // if (!(frame % encoderCounts)) digitalWrite(PWM_PIN, !digitalRead(PWM_PIN));
  OCR0B = pow(encoderCounts & 15, 2);
}

// Automagically gets called about 8,000 times per second
inline byte Audio(unsigned long t) {
  // This method of generating audio is known as "bytebeat" - generate samples one at a time from a continuously incrementing variable
  uint32_t q = t & 131071;
  return 34816>q?8*t|(t>>2)+13*t|(t>>3)%t|t>>5:67584>q?9*t|(t>>2)+13*t|(t>>3)%t|t>>5:83968>q?9*t|(t>>2)+12*t|(t>>3)%t|t>>5:98304>q?8*t|(t>>2)+12*t|(t>>3)%t|t>>5:8*t|(t>>2)+13*t|(t>>3)%t|t>>5;
}

// Automagically gets called any time an IO pin changes state
inline void IO(unsigned long deltaT, byte deltaB, byte deltaD) {
  if (deltaT < 1000) return;                                                        // Crappy debouncing
  if (digitalRead(ENC_SW) == LOW) {                                                 // Encoder button got pressed
    Serial.println(F("*snoot booping detected*"));
    Serial.print(F("Current encoder counts: "));
    Serial.println(encoderCounts);
  }
  if (deltaD & 0x14) {                                                              // Something happened on the encoder phase pins
    bool countDir = bool(deltaD & 0x10) ^ digitalRead(ENC_A) ^ digitalRead(ENC_B);  // If this is 1, the encoder turned clockwise; 0, counterclockwise
    encoderCounts += countDir ? 1 : -1;                                             // Update the encoder count accordingly
  }
}

// Automagically gets called whenever serial data is available
inline void SerialEvent() {
  while(Serial.available()) Serial.print((char)Serial.read());  // Echo back the received text
}
