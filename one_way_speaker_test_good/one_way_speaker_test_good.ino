#include <Wire.h>
#include <Adafruit_MCP4725.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE_PIN 7
#define CSN_PIN 8

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

Adafruit_MCP4725 dac;

void setup() {
  Serial.begin(115200);

  if (!dac.begin(0x62)) {
    Serial.println("DAC not found!");
    while (1);
  }
  Serial.println("DAC ready");

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_HIGH);
  radio.setDataRate(RF24_1MBPS);
  radio.startListening();
  Serial.println("Receiver ready");
}

void loop() {
  uint8_t audioPacket[16];

  if (radio.available()) {
    radio.read(&audioPacket, sizeof(audioPacket));
    Serial.print("Received: ");
    for (int i = 0; i < 16; i++) {
      uint16_t value12bit = audioPacket[i] * 16;
      dac.setVoltage(value12bit, false);
      Serial.print(audioPacket[i]);
      Serial.print(" ");
      delayMicroseconds(125);
    }
    Serial.println();
  }
}
