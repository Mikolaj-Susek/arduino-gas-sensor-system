
// Receiver Code

#include <RF24Network.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN
RF24Network network(radio);  

const uint16_t this_unit = 01;   // Address of our node in Octal format ( 04,031, etc)
const uint16_t central_unit = 00;

#define Threshold 700

#define gasSensor 5

int greenLed = 2;
int yellowLed = 3;
int redLed = 4;
int speaker = 5;

float sensorValue;  

// logic states
bool noSignalState = false; // lost signal with some sensor
bool alarmState = false; // found some gas

void setup() {
  // start serial monitor
  Serial.begin(9600);
  
  // start radio
  SPI.begin();
  radio.begin();
  network.begin(90, this_unit);
  radio.setDataRate(RF24_2MBPS);

  // set pins
  pinMode(greenLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  pinMode(speaker, OUTPUT);
}

void loop() {

  stateCheck();

  network.update();

  // RECEIVING

  while ( network.available() ){
    RF24NetworkHeader header;
    bool incomingData;
    network.read(header, &incomingData, sizeof(incomingData)); 
    
    if(header.from_node == 00){
      if (incomingData){ // bool reset signal from central_unit
        alarmState = false;
      } 
    }
  }

  
  delay(2000); // wait 2s for next reading 
}

void checkSensor(){
  sensorValue = analogRead(gasSensor); // read analog input pin 0
  
  Serial.print("Sensor Value: ");
  Serial.print(sensorValue);

  if(sensorValue > Threshold)
  {
    Serial.print(" | Smoke detected!");
    alarmState = true;
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



