//www.elegoo.com
//2016.12.8

// Do not need to Define Pins as they are defulat set to ouputs.


// Define rotary encoder
#define CLK 2
#define DT 3
#define SW 4

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;

int LEDValues[] = {0,0,0}; //R,G,B brightness
int LEDPins[] = {13,12,11}; //R,G,B pins
int currLED = 0;
int LED_brightness = 0;

int WantedLEDValues[] = {255,255,255}; //These will be a random value set by the NFC Tag
int WantedLEDPins[] = {8,9,10};

void setup()
{
//do not need to set pinmode to output for leds as it is already default

// Set encoder pins as inputs
pinMode(CLK,INPUT);
pinMode(DT,INPUT);
pinMode(SW, INPUT_PULLUP);

// Setup Serial Monitor
Serial.begin(9600);

// Read the initial state of CLK
lastStateCLK = digitalRead(CLK);

    analogWrite(LEDPins[0], LEDValues[0]);
    analogWrite(LEDPins[1], LEDValues[1]);
    analogWrite(LEDPins[2], LEDValues[2]);

    analogWrite(WantedLEDPins[0], WantedLEDValues[0]);
    analogWrite(WantedLEDPins[1], WantedLEDValues[1]);
    analogWrite(WantedLEDPins[2], WantedLEDValues[2]);
}

// main loop
void loop()
{
  int TrueVals = 0;
  for(int i=0; i<2; i++){
    if(LEDValues[i] == WantedLEDValues[i]){
      TrueVals++;
    }   
  }

  if(TrueVals == 3){
    Serial.println("The picked was locked!");
  }
// put your main code here, to run repeatedly:

  //roatery encoder
  // Read the current state of CLK
	currentStateCLK = digitalRead(CLK);

  //we use an if statement so that the arduino only changes its output if we give it an input
  
  // If last and current state of CLK are different, then pulse occurred
  // React to only 1 state change to avoid double count
  if (currentStateCLK != lastStateCLK && currentStateCLK == 1){
    
    //If the DT state is different than the CLK state then the econder is rotating counter-clockwise so decrement
    if (digitalRead(DT) != currentStateCLK) {
      counter--;
      //make sure brightness is a valid value
      if(LED_brightness > 0){
        LED_brightness = LED_brightness-1;
      }
      currentDir = "CCW";
    }
    else{
      //Encoder is rotating CW so increment
      counter++;
      //make sure brightness is a valid value
      if(LED_brightness < 255){
        LED_brightness = LED_brightness+1;
      }
      currentDir = "CW ";
    }

    //set the LED to the new brightness of the LED
    LEDValues[currLED] = LED_brightness;
    analogWrite(LEDPins[0], LEDValues[0]);
    analogWrite(LEDPins[1], LEDValues[1]);
    analogWrite(LEDPins[2], LEDValues[2]);


    //Print to serial monitor
    Serial.print("Direction: ");
		Serial.print(currentDir);
		Serial.print(" | Counter: ");
		Serial.print(counter);
    Serial.print(" | LED Brightness: ");
    Serial.println(LED_brightness);
  }  

  //save the current CLK state
  lastStateCLK = currentStateCLK;

  //check the button
  //read button state
  int btnState = digitalRead(SW);

  //If we detect a LOW signal, the button is pressed
  if (btnState == LOW){
    //if 50ms have passed since last low pulse(button pressed), it means that the button has been pressed, released and pressed again
    //millis() returns the number of milliseconds passwed since the Arduino began running the program. This will overflow to 0 after 50 days
    if(millis() - lastButtonPress > 50){
      Serial.println("Button Pressed!");
      currLED++;
      if(currLED > 2){
        currLED = 0;
      }
      LED_brightness = LEDValues[currLED];
      analogWrite(LEDPins[currLED], LED_brightness);
    }

    //Save last button press event
    lastButtonPress = millis();
  }

  //put in slight delay to debounce the reading
  delay(1);

}


