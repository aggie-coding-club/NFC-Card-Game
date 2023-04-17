//LED inputs
//LED is connected to pin 11 and ground
// be sure to put a resistor in series with the LED!
#define LED_PIN 11 //Basic LED
#define Basic_R 7 //State showing Red LED
#define Basic_G 8 //State showing Red LED
#define Basic_B 9 //State showing Red LED

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
#define R_led 5
#define G_led 6
#define B_led 10

// can also define these as constant integers

//more with roatery encoder
int counter = 0; // will increment each time the knob is rotated one detent (click)
int currentStateCLK; //we need these to store the state of the CLK output 
int lastStateCLK;    //and they are used to calculate amount of rotation
String currentDir =""; // used to output to serial monitor (click magnifying glass icon in top right of this IDE)
unsigned long lastButtonPress = 0; // used to debounce a switch

// this is the starting brightness of the LED, it can be any value in between 0 and 255
int LED_brightness = 100;
int LED_control = 0;
uint8_t LED_R = 100; //Type only allows 0-255
uint8_t LED_G = 100;
uint8_t LED_B = 100;

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_PIN, OUTPUT); // set led pin to be an output pin

  //RGB led setup
  pinMode(R_led, OUTPUT);
  pinMode(G_led, OUTPUT);
  pinMode(B_led, OUTPUT);
  //State LEDsetups
  pinMode(Basic_R, OUTPUT);
  pinMode(Basic_G, OUTPUT);
  pinMode(Basic_B, OUTPUT);  
  // Set encoder pins as inputs
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
	pinMode(SW, INPUT_PULLUP);

  // Setup Serial Monitor
	Serial.begin(9600); //9600 is a standard value for data   transfer

  // Read the initial state of CLK
	lastStateCLK = digitalRead(CLK);  

  //LED takes value from 0 to 255
  //set LED to the default brightness value
  analogWrite(LED_PIN, LED_brightness);
  analogWrite(R_led, LED_R);
  analogWrite(G_led, LED_G);
  analogWrite(B_led, LED_B);

  //Sets the state showing LEDs, they are only digital
  digitalWrite(Basic_R, 1);
  digitalWrite(Basic_G, 0);
  digitalWrite(Basic_B, 0);

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
      //RGB
      if(LED_control == 0){
        if(LED_R > 0){
          LED_R = LED_R-10;
        }
        else{
          LED_R = 0;
        }
      }
      else if(LED_control == 1){
        if(LED_G > 0){
          LED_G = LED_G-10;
        }
        else{
          LED_G = 0;
        }
        
      }
      else{
        if(LED_B > 0){
          LED_B = LED_B-10;
        }
        else{
          LED_B = 0;
        }
      }
    }
    else{
      //Encoder is rotating CW so increment
      counter++;
      //make sure brightness is a valid value
      if(LED_brightness < 245){
        LED_brightness = LED_brightness+10;
      }
      currentDir = "CW ";

      //RGB
      if(LED_control == 0){
        if(LED_R < 250){
          LED_R = LED_R+10;
        }
        else{
          LED_R = 250;
        }
      }
      else if(LED_control == 1){
        if(LED_G < 250){
          LED_G = LED_G+10;
        }
        else{
          LED_G = 250;
        }
        
      }
      else{
        if(LED_B < 250){
          LED_B = LED_B+10;
        }
        else{
          LED_B = 250;
        }
      }
    }

    //set the LED to the new brightness of the LED
    analogWrite(LED_PIN, LED_brightness);
    analogWrite(R_led, LED_R);
    analogWrite(G_led, LED_G);
    analogWrite(B_led, LED_B);

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
      //Swich state of RGB control
      if(LED_control < 2){
        LED_control = LED_control + 1;        
      }
      else{
        LED_control = 0;        
      }
      Serial.println("State changed");


      //Show state of system on control LEDs
      if(LED_control == 0){
        digitalWrite(Basic_R, 1);
        digitalWrite(Basic_G, 0);
        digitalWrite(Basic_B, 0);
      }
      else if(LED_control == 1){
        digitalWrite(Basic_R, 0);
        digitalWrite(Basic_G, 1);
        digitalWrite(Basic_B, 0);
      }
      else{
        digitalWrite(Basic_R, 0);
        digitalWrite(Basic_G, 0);
        digitalWrite(Basic_B, 1);
      }
    }

    //Save last button press event
    lastButtonPress = millis();
  }

  

  //put in slight delay to debounce the reading
  delay(1);

}
