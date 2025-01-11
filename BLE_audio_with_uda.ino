#include "BluetoothA2DPSink.h"
#include "driver/i2s.h"

// Bluetooth-Gerätename
const char* bluetoothDeviceName = "BMW e46";

// I2S Pins für UDA1334A
const int I2S_BCLK = 26;
const int I2S_LRC = 25;
const int I2S_DOUT = 27;

// Metadaten-Verarbeitung
String currentTitle = "";
String currentArtist = "";
unsigned long lastMetadataTime = 0;
const unsigned long metadataDelay = 500;

// Lautstärke-Einstellung
const int maxVolume = 127;

// Bluetooth A2DP Sink Objekt
BluetoothA2DPSink a2dp_sink;

// Bluetooth Metadaten Callback
void avrc_metadata_callback(uint8_t id, const uint8_t *text) {
  String textStr = String((char*)text);
  if (textStr.length() > 0) {
    switch (id) {
      case 1: currentTitle = textStr; break;
      case 2: currentArtist = textStr; break;
    }
    lastMetadataTime = millis();
  }
}

// Song-Informationen ausgeben
void printSongInfo() {
  if (millis() - lastMetadataTime > metadataDelay) {
    if (currentTitle != "" || currentArtist != "") {
      Serial.print("Aktueller Song: ");
      if (currentTitle != "") {
        Serial.print("\"" + currentTitle + "\"");
        if (currentArtist != "") Serial.print(" von ");
      }
      if (currentArtist != "") Serial.print(currentArtist);
      Serial.println();
      currentTitle = "";
      currentArtist = "";
    }
  }
}

// Verbindungstatus Callback
void connection_state_changed(esp_a2d_connection_state_t state, void *ptr) {
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
    Serial.println("Gerät verbunden: " + String(a2dp_sink.get_connected_source_name()));
    a2dp_sink.set_volume(maxVolume);
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
    Serial.println("Gerät getrennt");
    currentTitle = "";
    currentArtist = "";
  }
}

void setup() {
  Serial.begin(115200);

  // I2S Konfiguration für UDA1334A
  static const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  static const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  a2dp_sink.set_i2s_config(i2s_config);
  a2dp_sink.set_pin_config(pin_config);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  a2dp_sink.set_auto_reconnect(true);

  a2dp_sink.start(bluetoothDeviceName);
  Serial.println("Bluetooth A2DP Sink gestartet. Warte auf Verbindung...");
}

void loop() {
  printSongInfo();
  delay(100);
}
