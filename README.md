# Remal BLE Serial Library
## Overview
The **Remal BLE Serial** library allows you to use Shabakah (ESP32C3) as a BLE (Bluetooth Low Energy) Serial device. This library abstracts away the complex BLE code and allows you to interact with the BLE module as if it were a simple Serial interface.

### Features
- **BLE Serial Communication**: Easily send and receive data over BLE using a familiar Serial interface.
- **Buffered Transmission**: Accumulate multiple data chunks and send as a single BLE notification (ideal for logging).
- **No Heap Fragmentation**: Uses `const char*` instead of Arduino `String` for long-running stability.
- **Cross-Platform Compatibility**: Built for Shabakah (ESP32C3) but works with any ESP32C3 board.
- **NimBLE Backend**: Uses NimBLE for lower memory usage and improved stability over BlueDroid.

### Compatible Applications
You can use the following apps to view and interact with the data being sent and received:
- **iOS - Paid**: [BLE Terminal HM-10](https://apps.apple.com/us/app/ble-terminal-hm-10/id1398703795)
- **iOS - Free**: [Bluefruit Connect](https://apps.apple.com/us/app/bluefruit-connect/id830125974)
- **Android - Free**: [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US)

## Requirements
- **Hardware**: Shabakah (ESP32C3) or any other ESP32C3 board.
- **Software**: Arduino IDE with the **NimBLE-Arduino** library installed.

**Note**: If you have the Shabakah board installed in your Arduino IDE, you likely already have the required files. If not, follow this [Quick Start Guide](https://remal.io/quick-start/) to set up your environment. You can install NimBLE-Arduino via the Arduino Library Manager.

## Installation
### Arduino IDE
1. Download the latest release of the library from the [Releases](https://github.com/remalhq/Remal_BLE_Serial/releases) page.
2. Open the Arduino IDE and navigate to `Sketch` > `Include Library` > `Add .ZIP Library...`.
3. Select the downloaded ZIP file and click `Open`.
4. Install **NimBLE-Arduino** via `Sketch` > `Include Library` > `Manage Libraries...` and search for "NimBLE-Arduino".

## API Reference

### Basic Methods
| Method | Description |
|--------|-------------|
| `Init(DeviceName)` | Initialize BLE and start advertising with the given device name |
| `IsConnected()` | Returns `true` if a device is connected |
| `Send_Data(const char*)` | Send data to connected device (immediate or buffered) |
| `Data_Available()` | Returns number of messages in RX buffer, 0 if empty, -1 if disconnected |
| `Get_Data(char* Buffer, size_t BufferSize)` | Copies earliest message to buffer, returns bytes copied |
| `Set_RX_BufferSize(Size)` | Set RX buffer size (default: 128) |
| `Deinit()` | Deinitialize BLE and release resources |

### Buffering Methods (New in v2.0.0)
| Method | Description |
|--------|-------------|
| `Begin_Buffer()` | Start buffering mode - `Send_Data()` calls accumulate in buffer |
| `Flush_Buffer()` | Send buffered data as single notification and exit buffering mode |
| `Is_Buffering()` | Returns `true` if currently in buffering mode |
| `Clear_Buffer()` | Discard buffer contents and exit buffering mode |
| `Set_TX_BufferSize(Size)` | Set max TX buffer size (default: 512 bytes) |
| `Get_BufferLength()` | Returns current buffer content length |

## Usage Examples

### Basic Send/Receive
```cpp
#include <Remal_BLE_Serial.h>

BLESerial BT_Device;
char RX_Buffer[128];
char TX_Buffer[128];

void setup() {
    Serial.begin(115200);
    BT_Device.Init("Shabakah");
}

void loop() {
    // Send data
    if (BT_Device.IsConnected()) {
        BT_Device.Send_Data("Hello from Shabakah!\r\n");
    }

    // Receive data
    if (BT_Device.Data_Available() > 0) {
        int len = BT_Device.Get_Data(RX_Buffer, sizeof(RX_Buffer));
        if (len > 0) {
            Serial.print("Received: ");
            Serial.println(RX_Buffer);

            // Echo back
            snprintf(TX_Buffer, sizeof(TX_Buffer), "Echo: %s\r\n", RX_Buffer);
            BT_Device.Send_Data(TX_Buffer);
        }
    }

    delay(1000);
}
```

### Buffered Logging (Recommended for Multi-Part Messages)
When sending log messages that are built from multiple parts (colors, brackets, timestamps, etc.), use buffering to send them as a single BLE notification:

```cpp
#include <Remal_BLE_Serial.h>

BLESerial BT_Device;

void SendLogMessage(const char* Level, const char* Source, const char* Message) {
    BT_Device.Begin_Buffer();       // Start accumulating

    BT_Device.Send_Data("[");
    BT_Device.Send_Data(Level);
    BT_Device.Send_Data("] ");
    BT_Device.Send_Data(Source);
    BT_Device.Send_Data(": ");
    BT_Device.Send_Data(Message);
    BT_Device.Send_Data("\r\n");

    BT_Device.Flush_Buffer();       // Send as single notification
}

void loop() {
    SendLogMessage("INFO", "Sensor", "Temperature: 25C");
    delay(2000);
}
```

Without buffering, each `Send_Data()` would trigger a separate BLE notification, potentially causing errors and fragmented messages on the receiving end.

## Contributing
We welcome contributions! If you wish to contribute, please submit a pull request with a clear description of your changes.

## Changelog
### v2.0.0:
- Migrated from BlueDroid to NimBLE for improved stability and lower memory usage
- Changed API from Arduino `String` to `const char*` to avoid heap fragmentation
- Added buffering API (`Begin_Buffer`, `Flush_Buffer`, etc.) for sending complete messages
- Fixed issues with rapid notifications causing `BT_APPL` errors

### v1.0.1:
- Fixed bug in `RX_Callbacks` in the `onWrite()` function

### v1.0.0 - Git Release:
- Initial release to GitHub with Arduino IDE support.
