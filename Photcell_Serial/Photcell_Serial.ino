 // pin assignments
 int LDR = 0;
 
 // initialize the serial port
 // and declare inputs and outputs
 void setup() {
   pinMode(LDR, INPUT);
   Serial.begin(9600);
 }
 
 // read from the analog input connected to the LDR
 // and print the value to the serial port.
 // the delay is only to avoid sending so much data
 // as to make it unreadable.
 void loop() {
   int v = analogRead(LDR);
   Serial.println(v);
   delay(1000);
 }
