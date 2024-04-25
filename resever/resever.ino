
// Receiver Code

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";

int diodaLed = 3;

int stanDiody = 0;

int speaker = 5;


void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  pinMode(diodaLed, OUTPUT);
  pinMode(speaker, OUTPUT);
}

void loop() {
  if (radio.available()) {
    
    radio.read(&stanDiody, sizeof(stanDiody));
    if (stanDiody == 1) {
      digitalWrite(diodaLed, HIGH);
        tone(speaker, 440); // Gra ton o częstotliwości 440 Hz
   
    } else {
      digitalWrite(diodaLed, LOW);
      noTone(speaker);    // Zatrzymuje grę tonu
    }
  } 
}