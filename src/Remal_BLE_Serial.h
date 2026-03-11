/**
 * @file    Remal_BLE_Serial.h
 * @author  Khalid Mansoor AlAwadhi, Remal <Khalid@remal.io>
 * @date    11 Mar 2026 (Initial release - 24 June 2023)
 *
 * @brief   This library allows you to use Shabakah (ESP32C3) as a BLE (Bluetooth Low Energy) Serial device.
 * 			It abstracts away all the BLE code and allows you to use it as a Serial device.
 *
 * 			To view the data being sent and received, you can one of these apps:
 * 				- BLE Terminal HM-10 (iOS): https://apps.apple.com/us/app/ble-terminal-hm-10/id1398703795
 * 				- Serial Bluetooth Terminal (Android): https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US
 *
 * @note 	This library has been built and tested using the Shabakah (ESP32C3) board, but it should work with any ESP32C3 board.
 * 			It requires the NimBLE-Arduino library to be installed. You can install it via the Arduino Library Manager.
 *
 * @note 	This library uses NimBLE which is lighter and more stable than the default BlueDroid stack.
 *          Original reference: BLE_uart.ino example from ESP32 BLE Arduino library
 *              Video: https://www.youtube.com/watch?v=oCMOYS71NIU
 *              Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
 */
#ifndef _REMAL_BLE_SERIAL_H_
#define _REMAL_BLE_SERIAL_H_

#include "Arduino.h"

//<!- C++ Includes ->
#include <string>
#include <vector>

//<!- NimBLE Includes ->
#include <NimBLEDevice.h>


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
		 * @brief   Sets the maximum number of RX messages stored in the internal queue
		 *
		 * @param   Size: Maximum number of received messages to keep [Default: 128]
		 *          If the queue is already larger, oldest messages are dropped.
		 *
		 * @return  None
		 */
		void Set_RX_BufferSize(int Size);

		/*
		 * @brief   Checks if we have any string data available to read from the RX buffer
		 *
		 * @param   None
		 *
		 * @return  >0 -> Number of queued messages available
		 *           0 -> No queued data and device is connected
		 *          -1 -> No queued data and no device connected
		 */
		int Data_Available();

		/*
		 * @brief   Gets the earliest string in the RX buffer and copies it to the provided buffer
		 *
		 * @param   Buffer: Pointer to buffer where received data will be copied
		 * @param   BufferSize: Size of the buffer in bytes
		 *
		 * @return  Number of bytes copied, 0 if no data available, -1 if error
		 */
		int Get_Data(char* Buffer, size_t BufferSize);

		/*
		 * @brief   Send data to the connected BLE device
		 *          When buffering is enabled, data is accumulated until Flush_Buffer() is called.
		 *          Otherwise, data is sent immediately.
		 *
		 * @param   DataToSend: Null-terminated string to send to the connected BLE device
		 *
		 * @return  0 if successful, -1 if not connected to a device
		 */
		int Send_Data(const char* DataToSend);

		/*
		 * @brief   Deinitialize BLE
		 *
		 * @param   None
		 *
		 * @return  None
		 */
		void Deinit();

		/*
		 * @brief   Begin buffering mode - subsequent Send_Data() calls accumulate
		 *          in an internal buffer instead of sending immediately
		 *
		 * @param   None
		 *
		 * @return  None
		 */
		void Begin_Buffer();

		/*
		 * @brief   Flush the accumulated buffer as a single BLE notification
		 *          and exit buffering mode
		 *
		 * @param   None
		 *
		 * @return  0 if successful, -1 if not connected, -2 if buffer empty
		 */
		int Flush_Buffer();

		/*
		 * @brief   Check if currently in buffering mode
		 *
		 * @param   None
		 *
		 * @return  true if buffering, false otherwise
		 */
		bool Is_Buffering();

		/*
		 * @brief   Clear the buffer without sending and exit buffering mode
		 *
		 * @param   None
		 *
		 * @return  None
		 */
		void Clear_Buffer();

		/*
		 * @brief   Set maximum TX buffer size
		 *
		 * @param   Size: Maximum buffer size in bytes [Default size is 512]
		 *
		 * @return  None
		 */
		void Set_TX_BufferSize(size_t Size);

		/*
		 * @brief   Get current buffer content length
		 *
		 * @param   None
		 *
		 * @return  Number of bytes currently in buffer
		 */
		size_t Get_BufferLength();


	private:
		NimBLEServer *pServer = nullptr;                 // BLE Server
		NimBLECharacteristic *pTxCharacteristic = nullptr; // BLE TX Characteristic

		// TX Buffering
		std::string TX_Buffer;                           // Accumulates data when buffering
		bool BufferingEnabled = false;                   // Flag for buffering mode
		size_t TX_BufferMaxSize = 512;                   // Maximum buffer size (default 512)
};


#endif /* _REMAL_BLE_SERIAL_H_ */