// Transmiter

#include <RF24Network.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
RF24Network network(radio);   // include radio in network

const uint16_t this_unit = 0;
const uint16_t sensorsAddressTab[1] = {1}; // we can include another sensor_unit here

// button to reset alarm state and send information about reset to others unit
const int resetButtonPin = 3; 
int buttonCounter = 0;

int speaker = 2;


// logic state table
bool stateTab[3] = {false, false, false};

// led pins (4 - green, 5 - yellow, 6 - red)
int ledPinsTab[3] = {4,5,6};

int payload;


void setup() {
  // serial monitor start
  Serial.begin(9600);

  // start radio network
  SPI.begin();
  radio.begin();

  // communication channel same for every unit, address of this specific unit
  network.begin(90, this_unit);
  radio.setDataRate(RF24_2MBPS);

  // set pins

  pinMode(resetButtonPin, INPUT_PULLUP);

  for(int i=0; i < sizeof(ledPinsTab)/sizeof(int); i++){ // define led pins from table as a output
    pinMode(ledPinsTab[i], OUTPUT);
  }

  pinMode(speaker, OUTPUT); 

  changeStateTo(1); // good condition is set at the beginning
}

void loop() {

  network.update();
  
  // RECEIVING

  while( network.available() ){
    RF24NetworkHeader header;
    
    int incomingPayload; 
    network.read(header, &incomingPayload, sizeof(incomingPayload));
    
    /* 
    Check incoming massage 

    1 - GOOD
    2 - ALARM 
    */

    if (incomingPayload == 2){ // if any sensor send alarm we set alarm on central unit
      changeStateTo(3);   
    }

  }
    
  // TRANSMITING
  
  // send payload to other sensor units
  for(uint16_t address : sensorsAddressTab){
    RF24NetworkHeader header(address);
    bool result = network.write(header, &payload, sizeof(payload));
    
    if(!result){  // message dont reach address and alarm isn't set
      changeStateTo(2);           // change to no signal state
    }
  }


  int buttonState = digitalRead(resetButtonPin);

  if (buttonState == LOW){ // back to good state if button is clicked
    if (buttonCounter>3){
      Serial.println("Button clicked!");
      changeStateTo(1);
    } else {
      buttonCounter++;
    }
  } else {
    buttonCounter = 0;
  }

  delay(100);
  
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


    














