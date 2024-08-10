/**
 * @file    Remal_BLE_Serial.h
 * @author  Khalid Mansoor AlAwadhi, Remal <Khalid@remal.io>
 * @date    10 August 2024 (Initial release - 24 June 2023)
 * 
 * @brief   This library allows you to use Shabakah (ESP32C3) as a BLE (Bluetooth Low Energy) Serial device.
 * 			It abstracts away all the BLE code and allows you to use it as a Serial device.
 * 			
 * 			To view the data being sent and received, you can one of these apps:
 * 				- BLE Terminal HM-10 (iOS): https://apps.apple.com/us/app/ble-terminal-hm-10/id1398703795
 * 				- Serial Bluetooth Terminal (Android): https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US
 * 
 * @note 	This library has been built and tested using the Shabakah (ESP32C3) board, but it should work with any ESP32C3 board.
 * 			It also requires the ESP32 BLE Arduino library to be installed. If you have the Shabakah board installed in your Arduino IDE,
 * 			you should already have the needed files. 
 * 			If not, you can follow this guide to install it here: https://remal.io/quick-start/
 * 
 * @note 	This library has been built using the BLE_uart.ino example as a reference from the ESP32 BLE Arduino library
 *          Notes kept from example:
 *              Video: https://www.youtube.com/watch?v=oCMOYS71NIU
 *              Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
 *              Ported to Arduino ESP32 by Evandro Copercini
 */
#ifndef _REMAL_BLE_SERIAL_H_
#define _REMAL_BLE_SERIAL_H_

#include "Arduino.h"

//<!- C++ Includes ->
#include <string>
#include <vector>

//<!- Arduino Includes ->
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


/***********************************
 * <!- Defines ->
 ***********************************/
// UUIDs based on Nordic UART Service
// New UUIDs can be generated using: https://www.uuidgenerator.net/
#define UART_SERVICE_UUID                "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"     // UUID for the BLE UART (Serial) service
#define UART_CHARACTERISTIC_UUID_TX      "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"     // UUID for the TX characteristic
#define UART_CHARACTERISTIC_UUID_RX      "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"     // UUID for the RX characteristic


class BLESerial
{
	public:
		/* 
		 * @brief   Setups BLE and starts advertising
		 * 
		 * @param   DeviceName: The name of the device that will be shown when scanning for BLE devices
		 * 
		 * @return  None
		 */
		void Init(const char* DeviceName);

		/* 
		 * @brief   Checks if we are connected to a device
		 * 
		 * @param   None
		 * 
		 * @return  Returns true if connected to a device, false if not
		 */
		bool IsConnected();

		/* 
		 * @brief   Sets the size of the RX buffer
		 * 
		 * @param   Size: The size of the RX buffer [Default size is 128]
		 * 
		 * @return  None
		 */
		void Set_RX_BufferSize(int Size);

		/* 
		 * @brief   Checks if we have any string data available to read from the RX buffer
		 * 
		 * @param   None
		 * 
		 * @return  0 -> No data available | -1 -> Error, no device connected | Any number greater than zero -> Number of strings available to read
		 */
		int Data_Available();

		/* 
		 * @brief   Gets the earliest string in the RX buffer
		 * 
		 * @param   None
		 * 
		 * @return  Returns the earliest string in the RX buffer or an empty string if no data is available
		 */
		String Get_Data();

		/* 
		 * @brief   Send data to the connected BLE device
		 * 
		 * @param   DataToSend: String that contains data to send to the connected BLE device
		 * 
		 * @return  0 if successful, -1 if not connected to a device
		 */
		int Send_Data(String DataToSend);

		/* 
		 * @brief   Deinitialize BLE
		 * 
		 * @param   None
		 * 
		 * @return  None 
		 */
		void Deinit();
		

	private:
		BLEServer *pServer;                         // BLE Server
		BLECharacteristic *pTxCharacteristic;       // BLE Characteristic
};


#endif /* _REMAL_BLE_SERIAL_H_ */