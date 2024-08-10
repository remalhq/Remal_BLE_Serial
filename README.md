# Remal BLE Serial Library
## Overview
The **Remal BLE Serial** library allows you to use Shabakah (ESP32C3) as a BLE (Bluetooth Low Energy) Serial device. This library abstracts away the complex BLE code and allows you to interact with the BLE module as if it were a simple Serial interface.

### Features
- **BLE Serial Communication**: Easily send and receive data over BLE using a familiar Serial interface.
- **Cross-Platform Compatibility**: Built for Shabakah (ESP32C3) but works with any ESP32C3 board.
- **Seamless Integration**: Utilizes the ESP32 BLE Arduino library, with minimal setup required.

### Compatible Applications
You can use the following apps to view and interact with the data being sent and received:
- **iOS**: [BLE Terminal HM-10](https://apps.apple.com/us/app/ble-terminal-hm-10/id1398703795)
- **Android**: [Serial Bluetooth Terminal](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US)

## Requirements
- **Hardware**: Shabakah (ESP32C3) or any other ESP32C3 board.
- **Software**: Arduino IDE with the ESP32 BLE Arduino library installed.

**Note**: If you have the Shabakah board installed in your Arduino IDE, you likely already have the required files. If not, follow this [Quick Start Guide](https://remal.io/quick-start/) to set up your environment.

## Installation
### Arduino IDE
1. Download the latest release of the library from the [Releases](https://github.com/remalhq/Remal_BLE_Serial/releases) page.
2. Open the Arduino IDE and navigate to `Sketch` > `Include Library` > `Add .ZIP Library...`.
3. Select the downloaded ZIP file and click `Open`.

## Contributing
We welcome contributions! If you wish to contribute, please submit a pull request with a clear description of your changes.

## Changelog
### v1.0.0 - Git Release:
- Initial release to GitHub with Arduino IDE support.