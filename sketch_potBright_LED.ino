//LED inputs
//LED is connected to pin 11 and ground
// be sure to put a resistor in series with the LED!
#define LED_PIN 11

// Rotary Encoder Inputs
// Roatery encoder is connected with
// + = 5V
// GND = GND
// CLK = pin 2
// DT = pin ~3
// SW = pin 4
#define CLK 2
#define DT 3
#define SW 4
// can also define these as constant integers

//more with roatery encoder
int counter = 0; // will increment each time the knob is rotated one detent (click)
int currentStateCLK; //we need these to store the state of the CLK output 
int lastStateCLK;    //and they are used to calculate amount of rotation
String currentDir =""; // used to output to serial monitor (click magnifying glass icon in top right of this IDE)
unsigned long lastButtonPress = 0; // used to debounce a switch

// this is the starting brightness of the LED, it can be any value in between 0 and 255
int LED_brightness = 100;

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_PIN, OUTPUT); // set led pin to be an output pin

  // Set encoder pins as inputs
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
	pinMode(SW, INPUT_PULLUP);

  // Setup Serial Monitor
	Serial.begin(9600); //9600 is a standard value for data transfer

  // Read the initial state of CLK
	lastStateCLK = digitalRead(CLK);  

  //LED takes value from 0 to 255
  //set LED to the default brightness value
  analogWrite(LED_PIN, LED_brightness);

}

void loop() {
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
      if(LED_brightness > 10){
        LED_brightness = LED_brightness-10;
      }
      currentDir = "CCW";
    }
    else{
      //Encoder is rotating CW so increment
      counter++;
      //make sure brightness is a valid value
      if(LED_brightness < 245){
        LED_brightness = LED_brightness+10;
      }
      currentDir = "CW ";
    }

    //set the LED to the new brightness of the LED
    analogWrite(LED_PIN, LED_brightness);

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
      //set brightness to 0
      LED_brightness = 0;
      analogWrite(LED_PIN, LED_brightness);
    }

    //Save last button press event
    lastButtonPress = millis();
  }

  //put in slight delay to debounce the reading
  delay(1);

}
