#include <Arduino.h>
#include <SPI.h>
#include <RF24.h>
#include "driver/i2s.h"

// nRF24 wiring
#define CE_PIN   9
#define CSN_PIN  21

RF24 radio(CE_PIN, CSN_PIN);
const byte address[6] = "00001";

// I²S pins & params
#define I2S_BCLK   10      // GPIO10
#define I2S_LRC    17      // GPIO17
#define I2S_DOUT   18      // GPIO18
#define FS         16000U  // must match transmitter
#define BUF_LEN    16      // must match transmitter

static int16_t audioBuf[BUF_LEN];

void initI2S() {
  i2s_config_t cfg = {
    .mode                 = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate          = FS,
    .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format       = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S_MSB,
    .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count        = 2,
    .dma_buf_len          = BUF_LEN
  };
  i2s_pin_config_t pins = {
    .bck_io_num   = I2S_BCLK,
    .ws_io_num    = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num  = I2S_PIN_NO_CHANGE
  };
  i2s_driver_install(I2S_NUM_0, &cfg, 0, nullptr);
  i2s_set_pin(I2S_NUM_0, &pins);
}

void setup() {
  Serial.begin(115200);

  // init nRF24
  radio.begin();
  radio.setDataRate(RF24_2MBPS);
  radio.openReadingPipe(0, address);
  radio.startListening();

  // init I²S
  initI2S();
}

void loop() {
  if (radio.available()) {
    // receive raw 12-bit samples
    uint16_t rawBuf[BUF_LEN];
    radio.read(rawBuf, sizeof(rawBuf));

    // convert to signed 16-bit PCM and play
    for (int i = 0; i < BUF_LEN; i++) {
      audioBuf[i] = ((int)rawBuf[i] - 2048) << 4;
    }
    size_t written;
    i2s_write(I2S_NUM_0,
              audioBuf,
              BUF_LEN * sizeof(int16_t),
              &written,
              portMAX_DELAY);
  }
}
