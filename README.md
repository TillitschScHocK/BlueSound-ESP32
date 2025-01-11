# ESP32 Bluetooth Audio Receiver with UDA1334 DAC Module

This project, named **BlueSound ESP**, demonstrates how to use an **ESP32 Devkit V4** along with a **UDA1334 DAC module** to create a Bluetooth audio receiver. The device receives audio via Bluetooth (A2DP protocol) and outputs sound through the AUX jack of the UDA1334 module. This setup is ideal for retrofitting older cars to support wireless audio streaming.

---

## Features
- **Bluetooth A2DP audio streaming**
- **I2S audio output** via UDA1334 DAC module
- **Song metadata display** (e.g., current track title and artist)
- **Auto-reconnect** to previously paired devices
- **Volume control** on connection

---

## Components
| Component       | Description                        |
|-----------------|------------------------------------|
| ESP32 Devkit V4 | Microcontroller with built-in Bluetooth support |
| UDA1334         | Low-power stereo DAC for high-quality audio |
| AUX cable       | Connects the UDA1334 module to your car's AUX input |
| Power supply    | 5V USB power supply for the ESP32 |

---

## Wiring Diagram
| ESP32 Pin | UDA1334 Pin |
|-----------|-------------|
| GPIO 26   | BCK         |
| GPIO 25   | LRC         |
| GPIO 27   | DOUT        |
| GND       | GND         |
| 3.3V      | VIN         |

---

## Code Explanation
The code sets up the ESP32 as a Bluetooth A2DP sink that outputs audio via I2S to the UDA1334 DAC module.

### Key Functions:
- `avrc_metadata_callback`: Handles incoming metadata such as track title and artist name.
- `connection_state_changed`: Manages connection state changes (e.g., connected/disconnected).
- `printSongInfo`: Displays the current song information via the serial monitor.

### Configuration:
- **Bluetooth Device Name**: `BMW e46` (change this to your desired device name).
- **I2S Pins**: GPIO 26 (BCK), GPIO 25 (LRC), GPIO 27 (DOUT).
- **Volume Control**: The `maxVolume` variable sets the maximum volume (default: 127).

---

## Code
```cpp
#include "BluetoothA2DPSink.h"
#include "driver/i2s.h"

const char* bluetoothDeviceName = "BMW e46";
const int I2S_BCLK = 26;
const int I2S_LRC = 25;
const int I2S_DOUT = 27;

String currentTitle = "";
String currentArtist = "";
unsigned long lastMetadataTime = 0;
const unsigned long metadataDelay = 500;

const int maxVolume = 127;
BluetoothA2DPSink a2dp_sink;

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

void printSongInfo() {
  if (millis() - lastMetadataTime > metadataDelay) {
    if (currentTitle != "" || currentArtist != "") {
      Serial.print("Current Song: ");
      if (currentTitle != "") {
        Serial.print("\"" + currentTitle + "\"");
        if (currentArtist != "") Serial.print(" by ");
      }
      if (currentArtist != "") Serial.print(currentArtist);
      Serial.println();
      currentTitle = "";
      currentArtist = "";
    }
  }
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr) {
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
    Serial.println("Device connected: " + String(a2dp_sink.get_connected_source_name()));
    a2dp_sink.set_volume(maxVolume);
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
    Serial.println("Device disconnected");
    currentTitle = "";
    currentArtist = "";
  }
}

void setup() {
  Serial.begin(115200);

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
  Serial.println("Bluetooth A2DP Sink started. Waiting for connection...");
}

void loop() {
  printSongInfo();
  delay(100);
}
```

---

## How to Use
1. **Connect the components** as per the wiring diagram.
2. **Upload the code** to your ESP32 using the Arduino IDE.
3. **Pair your Bluetooth device** (e.g., smartphone) with the ESP32.
4. **Play audio** on your paired device. The sound will output through the UDA1334's AUX jack.

---

## Notes
- The UDA1334 module outputs a line-level signal. For best results, connect it to an amplifier or a car AUX input.
- You can change the Bluetooth device name by modifying the `bluetoothDeviceName` variable in the code.


---

## License
This project is licensed under the MIT License. See the LICENSE file for details.

