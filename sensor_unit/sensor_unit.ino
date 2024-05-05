
// Receiver Code

#include <RF24Network.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
RF24Network network(radio);  

const uint16_t this_unit = 01;   // Address of our node in Octal format ( 04,031, etc)
const uint16_t central_unit = 00; 

#define Threshold 700 // upper limit of sensor when it can find gas

#define gasSensor 5

int payload;

int speaker = 5;

float sensorValue;  


// logic state table
bool stateTab[3] = {false, false, false};

// led pins (2 - green, 3 - yellow, 4 - red)
int ledPinsTab[3] = {2,3,4};


void setup() {
  // start serial monitor
  Serial.begin(9600);
  
  // start radio
  SPI.begin();
  radio.begin();

  // communication channel same for every unit, address of this specific unit
  network.begin(90, this_unit);
  radio.setDataRate(RF24_2MBPS);

  // set pins
  for(int i=0; i < sizeof(ledPinsTab)/sizeof(int); i++){ // define led pins from table as a output
    pinMode(ledPinsTab[i], OUTPUT);
  }

  pinMode(speaker, OUTPUT);
}

void loop() {

  // check gas sensor
  // if (isGasToHigh()){
  //   changeStateTo(3); // set alarm state
  // }

  network.update();

  // RECEIVING

  while ( network.available() ){
    RF24NetworkHeader header;
    
    int incomingPayload; 
    network.read(header, &incomingPayload, sizeof(incomingPayload));
    
    if(header.from_node == central_unit){
      Serial.println("Get message from central_unit");
      switch(incomingPayload){
        case 1:               // reset to good message
          changeStateTo(1);   // set good state
          break;
        case 2:               // alarm message
          changeStateTo(3);   // set alarm state
      }
    }
  }

  // TRANSMITING

  RF24NetworkHeader header(central_unit);
  bool result = network.write(header, &payload, sizeof(payload));
    
  if(!result){  // message dont reach address and alarm isn't set
    Serial.println("Failed to send message");
    changeStateTo(2);           // change to no signal state
  }

  
  
}

bool isGasToHigh(){
  sensorValue = analogRead(gasSensor); // read analog input pin 0
  
  Serial.println("Sensor Value: ");
  Serial.println(sensorValue);

  if(sensorValue > Threshold)
  {
    Serial.print(" | Smoke detected!");
    return true;
  }
  return false;
  
}


void reset(){
  for(int i=0; i < sizeof(stateTab)/sizeof(bool); i++){
    stateTab[i] = false;              // reset states
    digitalWrite(ledPinsTab[i], LOW); // turn off leds
    noTone(speaker);                  // turn off speaker
  }
} 

void changeStateTo(int stateNumber){

  /*
  Function to simplify state changes. We only need to know which number of every state.
  1 - GOOD
  2 - NOSINGAL
  3 - ALARM
  */


  if (stateNumber < 1 && stateNumber > 3){
    Serial.println("Feild to change state. Number out of scope.");
  }
   

  switch (stateNumber) {
    case 1: // goodState
      if(!stateTab[0]){
        reset();
        stateTab[0] = true;
        digitalWrite(ledPinsTab[0], HIGH);
        payload = 1;
        Serial.println("Change to good state");         
      }
      break;

    case 2: // noSignalState
      if(!stateTab[1] && !stateTab[2]){
        reset();
        stateTab[1] = true;
        digitalWrite(ledPinsTab[1], HIGH);
        payload = 1;
        Serial.println("Change to no signal");
      }
      break;

    case 3: // alarmState
      if(!stateTab[2]){ 
        reset(); 
        stateTab[2] = true;
        digitalWrite(ledPinsTab[2], HIGH);
        tone(speaker, 3000);
        payload = 2;
        Serial.println("Change to alarm");
      }
      break;
  }
}






