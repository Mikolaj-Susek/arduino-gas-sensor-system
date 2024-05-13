// Transmiter


#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte ownAddress = "00001";
const byte sensorsAddressTab[1] = {"00002"}; // we can include another sensor_unit here

// button to reset alarm state and send information about reset to others unit
const int resetButtonPin = 3; 
int buttonCounter = 0;
bool startReset = false;

int speaker = 2;

int payload;

// logic state table
bool stateTab[3] = {false, false, false};

// led pins (4 - green, 5 - yellow, 6 - red)
int ledPinsTab[3] = {4,5,6};

const byte addresses[][6] = {"00001", "00002"};

// package handling
int failedPackageCounter = 0;
int failedPackageLimit = 3;

void setup() {
  // serial monitor start
  Serial.begin(9600);

  // start radio network
  radio.begin();

  // communication channel same for every unit, address of this specific unit
  radio.openWritingPipe(addresses[1]); // 00002
  radio.openReadingPipe(1, addresses[0]); // 00001
  // radio.openReadingPipe(1, ownAddress); // 00001
  radio.setPALevel(RF24_PA_MIN);

  // set pins

  pinMode(resetButtonPin, INPUT_PULLUP);

  for(int i=0; i < sizeof(ledPinsTab)/sizeof(int); i++){ // define led pins from table as a output
    pinMode(ledPinsTab[i], OUTPUT);
  }

  pinMode(speaker, OUTPUT); 

  changeStateTo(1); // good condition is set at the beginning
}

void loop() {

  int buttonState = digitalRead(resetButtonPin);

  if (buttonState == LOW){ // back to good state if button is clicked
    if (buttonCounter>5){
      Serial.println("Button clicked!");
      changeStateTo(1);
      startReset = true;
    } else {
      buttonCounter++;
    }
  } else {
    buttonCounter = 0;
    startReset = false;
  }
  
  // RECEIVING
  radio.startListening();
  if( radio.available()){
    
    int incomingPayload; 
    radio.read(&incomingPayload, sizeof(incomingPayload));
    
    /* 
    Check incoming massage 

    1 - GOOD
    2 - ALARM 
    */

    if (incomingPayload == 2){ // if any sensor send alarm we set alarm on central unit
      if(!startReset){
        changeStateTo(3);
      }
    }
    
  } delay(60);

  radio.stopListening();
    
  // TRANSMITING
  
  // send payload to other sensor units
  // for(byte address : sensorsAddressTab){
  //   radio.openWritingPipe(address);
    bool result = radio.write(&payload, sizeof(payload));
    
    if(!result){  // message dont reach address and alarm isn't set
      if (failedPackageCounter>failedPackageLimit){
        Serial.println("Failed to send message");
        changeStateTo(2);           // change to no signal state
      } else {
        failedPackageCounter++;
      }
    } else {
      Serial.println("Success send");
      failedPackageCounter=0;
      if (!stateTab[2]){      // if communitation is good and alarm isnt set.
        changeStateTo(1);     // Set good state
      }
    }
  // }



  
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