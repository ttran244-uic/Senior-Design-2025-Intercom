#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define MIC_PIN A0  // MAX4466 Mic input
#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

#define FILTER_SIZE 4
int filterBuffer[FILTER_SIZE] = {0};
int filterIndex = 0;

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
  radio.stopListening();
  Serial.println("Sender ready");
}

void loop() {
  uint8_t audioPacket[16];

  for (int i = 0; i < 16; i++) {
    int micValue = analogRead(MIC_PIN);
    micValue = map(micValue, 0, 1023, 0, 255);

    // Apply moving average filter
    filterBuffer[filterIndex] = micValue;
    filterIndex = (filterIndex + 1) % FILTER_SIZE;

    int filteredValue = 0;
    for (int j = 0; j < FILTER_SIZE; j++) {
      filteredValue += filterBuffer[j];
    }
    filteredValue /= FILTER_SIZE;

    audioPacket[i] = filteredValue;

    Serial.print(filteredValue);
    Serial.print(" "); // Optional: for CSV-style viewing

    delayMicroseconds(125); // 8kHz sampling
  }

  Serial.println(); // Newline per packet
  radio.write(audioPacket, sizeof(audioPacket));
  Serial.println("Packet sent");
}
