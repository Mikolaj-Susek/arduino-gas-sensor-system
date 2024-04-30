
// Transmiter

#include <RF24Network.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
RF24Network network(radio);   // include radio in network

const uint16_t this_unit = 00;
const uint16_t sensor_unit = 01; // we can include another sensor_unit here

// button to reset alarm state and send information about reset to others unit
const int resetButtonPin = 3; 
int stateOfResetButton = 0;

// led pins
int greenLed = 4;
int yellowLed = 5;
int redLed = 6;

int speaker = 2;

// logic states
bool noSignalState = false; // lost signal with some sensor
bool alarmState = false; // found some gas

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

  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  pinMode(speaker, OUTPUT);

  digitalWrite(greenLed, HIGH); // greenLed is set on start
}

void loop() {

  stateCheck();

  network.update();
  
  // RECEIVING

  while( network.available() ){
    RF24NetworkHeader header;
    
    bool incomingData; 
    network.read(header, &incomingData, sizeof(incomingData));
    
    // read boolean value from sensor about alarm state

    if (incomingData){
      alarmState = true;
    }
  }

  if ( digitalRead(resetButtonPin) ) { // disable states if reset button is pressed
    bool noSignalState = false; 
    bool alarmState = false;

    // TODO: need to send some information to reset alarm on sensor
  }
    
}


void stateCheck(){

  if (alarmState) {

    digitalWrite(greenLed,LOW);    
    digitalWrite(yellowLed,LOW);
    digitalWrite(redLed, HIGH);

    tone(speaker, 3000);
    
  } else if(noSignalState){ 

    digitalWrite(greenLed, LOW);
    digitalWrite(yellowLed, HIGH);
    digitalWrite(redLed, LOW);

    noTone(speaker);

  } else {

    digitalWrite(greenLed, HIGH);
    digitalWrite(yellowLed, LOW);
    digitalWrite(redLed, LOW);

    noTone(speaker);
  }
}








