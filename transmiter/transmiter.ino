
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

const int przyciskPin = 6; 

int stanPrzycisku = 0; 

int payload = 0;

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  pinMode(przyciskPin, INPUT_PULLUP);
}

void loop() {
  stanPrzycisku = digitalRead(przyciskPin);

  if (stanPrzycisku == LOW) {
    payload = 1; 
    radio.write(&payload, sizeof(int));
  } else {
    payload = 0;
    radio.write(&payload, sizeof(int));
  }
  delay(5);
}





