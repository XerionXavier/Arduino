const int firstLED = 3;      // pin for the LED
const int inputPin = 2;      // input pin for the PIR sensor
int PIRstate;               // variable to hold the last PIR state
int val;                    // variable for reading the pin status

void setup() {

  pinMode(inputPin, INPUT);              //declare PIR as input
  PIRstate = digitalRead(inputPin);      //assign PIR state to PIRstate
}

void loop() { 

  val = digitalRead(inputPin);      // read input value 
  if (val != PIRstate)                      // check if the input has changed
  {
    if(val == HIGH){  
      analogWrite(firstLED, 255);
    } 
    else{

      for(int fadeValue = 255 ; fadeValue >= 0; fadeValue -=5) { 
        // sets the value (range from 0 to 255):
        analogWrite(firstLED, fadeValue);         
        // wait for 30 milliseconds to see the dimming effect    
        delay(90);                            
      }
    }
  }
  PIRstate = val;                 // save the new state to the variable
}
