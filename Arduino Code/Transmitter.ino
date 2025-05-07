#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>

// nRF24 wiring
#define CE_PIN   9
#define CSN_PIN 21

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// Mic & sampling parameters
#define MIC_PIN A3
#define FS      16000UL  // sample rate
#define BUF_LEN 16       // 16 samples per packet (32 bytes payload)

static uint16_t buf[BUF_LEN];
static uint8_t  idx       = 0;
static unsigned long nextMicros;

void setup() {
  Serial.begin(115200);

  // init nRF24
  radio.begin();
  radio.setDataRate(RF24_2MBPS);
  radio.openWritingPipe(address);
  radio.stopListening();

  // mic ADC
  analogReadResolution(12);
  analogSetPinAttenuation(MIC_PIN, ADC_11db);
  nextMicros = micros();
}

void loop() {
  // sample at FS
  if ((long)(micros() - nextMicros) >= 0) {
    nextMicros += 1000000UL / FS;
    buf[idx++] = analogRead(MIC_PIN);
    if (idx >= BUF_LEN) {
      // send raw 12-bit samples
      radio.write(buf, sizeof(buf));
      idx = 0;
    }
  }
}
