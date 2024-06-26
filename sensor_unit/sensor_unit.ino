// Receiver Code

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte own_address = "00002";   // Address of our node in Octal format ( 04,031, etc)
const byte central_address = "00001"; 

#define Threshold 450 // Upper limit of sensor when it can detect gas

#define gasSensor 5

int payload;

int speaker = 5;

float sensorValue;  

// Logic state table
bool stateTab[3] = {false, false, false};

// LED pins (2 - green, 3 - yellow, 4 - red)
int ledPinsTab[3] = {2,3,4};

const byte addresses[][6] = {"00001", "00002"};

// Package handling
int failedPackageCounter = 0;
int failedPackageLimit = 3;

void setup() {
  // Start serial monitor
  Serial.begin(9600);
  
  // Start radio
  radio.begin();

  // Communication channel same for every unit, address of this specific unit
  // radio.openWritingPipe(central_address); // 00001
  // radio.openReadingPipe(1, own_address);  // 00002
  radio.openWritingPipe(addresses[0]); // 00001
  radio.openReadingPipe(1, addresses[1]); // 00002
  
  radio.setPALevel(RF24_PA_MIN);

  // Set pins
  for(int i=0; i < sizeof(ledPinsTab)/sizeof(int); i++){ // Define LED pins from table as an output
    pinMode(ledPinsTab[i], OUTPUT);
  }

  pinMode(speaker, OUTPUT);

  changeStateTo(1);
}

void loop() {

  radio.startListening();

  // RECEIVING

  if ( radio.available()){
    
    int incomingPayload; 
    radio.read(&incomingPayload, sizeof(incomingPayload));
    
    switch(incomingPayload){
      case 1:               // Reset to good message
        changeStateTo(1);   // Set good state
          break;
      case 2:               // Alarm message
        changeStateTo(3);   // Set alarm state
    }
  } delay(60);
  

  radio.stopListening();

  // Check gas sensor
  if (isGasToHigh()){
    changeStateTo(3); // Set alarm state
  }

  // TRANSMITTING

  
  bool result = radio.write(&payload, sizeof(payload));
    
  if(!result){  // Message didn't reach address and alarm isn't set
    if ( failedPackageCounter > failedPackageLimit ){
    
      Serial.print("|\tFailed to send message");
    
      changeStateTo(2);           // Change to no signal state
    
    } else {
    
      failedPackageCounter++;
    
    }
  } else {
    Serial.print("| Success send\n");
    
    failedPackageCounter=0;
    
    if (!stateTab[2]){      // If communication is good and alarm isn't set.
      changeStateTo(1);     // Set good state
    }
  }

  
  
}

bool isGasToHigh(){
  sensorValue = analogRead(gasSensor); // Read analog input pin 0
  
  Serial.print("Sensor Value: ");
  Serial.print(sensorValue);

  if(sensorValue > Threshold)
  {
    Serial.print("|\tSmoke detected!");
    return true;
  }
  Serial.print("|\tNo smoke");
  return false;
  
}


void reset(){
  for(int i=0; i < sizeof(stateTab)/sizeof(bool); i++){
    stateTab[i] = false;              // Reset states
    digitalWrite(ledPinsTab[i], LOW); // Turn off LEDs
    noTone(speaker);                  // Turn off speaker
  }
} 

void changeStateTo(int stateNumber){

  /*
  Function to simplify state changes. We only need to know which number of every state.
  1 - GOOD
  2 - NO SIGNAL
  3 - ALARM
  */


  if (stateNumber < 1 && stateNumber > 3){
    Serial.println("Failed to change state. Number out of scope.");
  }
   

  switch (stateNumber) {
    case 1: // Good state
      if(!stateTab[0]){
        reset();
        stateTab[0] = true;
        digitalWrite(ledPinsTab[0], HIGH);
        payload = 1;
        Serial.print("|\tChange to good state");         
      }
      break;

    case 2: // No signal state
      if(!stateTab[1] && !stateTab[2]){
        reset();
        stateTab[1] = true;
        digitalWrite(ledPinsTab[1], HIGH);
        payload = 1;
        Serial.print("|\tChange to no signal");
      }
      break;

    case 3: // Alarm state
      if(!stateTab[2]){ 
        reset(); 
        stateTab[2] = true;
        digitalWrite(ledPinsTab[2], HIGH);
        tone(speaker, 3000);
        payload = 2;
        Serial.print("|\tChange to alarm");
      }
      break;
  }
}
