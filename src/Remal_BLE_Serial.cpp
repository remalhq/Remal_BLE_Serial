/*
 * Remal_BLE_Serial.cpp
 *
 *  # ALL INFO CAN BE FOUND IN THE .h FILE #
 */
#include "Remal_BLE_Serial.h"


/***********************************
 * Private variables
 ***********************************/
static volatile bool DeviceConnected = false;		// Flag to indicate if a device is connected
static std::vector<std::string> BT_RX_Buffer;		// RX Buffer


/***************************************************************
 * 				Private callback functions
 ***************************************************************/
/* 
 * @brief   This callback is called when a device connects or disconnects
 */
class Server_Callbacks: public BLEServerCallbacks 
{
	void onConnect(BLEServer* pServer)
	{
		DeviceConnected = true;
	};

	void onDisconnect(BLEServer* pServer)
	{
		DeviceConnected = false;

		/* Restart advertising */
		delay(500);
		pServer->startAdvertising();
	}
};

/* 
 * @brief   This callback is called when the RX characteristic is written to
 */
class RX_Callbacks: public BLECharacteristicCallbacks 
{
   	void onWrite(BLECharacteristic *pCharacteristic) 
	{
   		std::string RX_Value = pCharacteristic->getValue();

		/* Store the received data in the RX buffer */
		BT_RX_Buffer.push_back(RX_Value);
	}
};



/***************************************************************
 * 				Class Functions
 ***************************************************************/
void BLESerial::Init(const char* DeviceName)
{
	BLEDevice::init(DeviceName);								// Initialize BLE device
	pServer = BLEDevice::createServer();						// Create the BLE Server
	pServer->setCallbacks(new Server_Callbacks());				// Set the server callbacks

	// Create the BLE Service (UART)
	BLEService *pService = pServer->createService(UART_SERVICE_UUID);		

	// Create BLE TX and RX Characteristics (the TX/RX namings are from the client's perspective)
	pTxCharacteristic = pService->createCharacteristic( UART_CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY );
	pTxCharacteristic->addDescriptor(new BLE2902());
	BLECharacteristic * pRxCharacteristic = pService->createCharacteristic( UART_CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_WRITE );
	pRxCharacteristic->setCallbacks(new RX_Callbacks());		// Set the RX callback - This is called when the characteristc 'UART_CHARACTERISTIC_UUID_RX' is written to

	// Set the size of the RX buffer and clear contents
	BT_RX_Buffer.reserve(128);
	BT_RX_Buffer.clear();

	// Start the service and advertising
	pService->start();					
	pServer->getAdvertising()->start();
}



bool BLESerial::IsConnected()
{
	return DeviceConnected;
}



void BLESerial::Set_RX_BufferSize(int Size)
{
	/* Error checking: Make sure size is not zero or negative */
	if(Size <= 0)
	{
		return;
	}
	BT_RX_Buffer.resize(Size);
}



int BLESerial::Data_Available()
{
	if(DeviceConnected)
	{
		/* Check if we have any data available to read */
		if( !BT_RX_Buffer.empty() )
		{
			return BT_RX_Buffer.size();
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return -1;
	}
}



String BLESerial::Get_Data()
{
	/* Check if we have any data available to read */
	if( !BT_RX_Buffer.empty() )
	{
        std::string DataElement = BT_RX_Buffer.front();		// Get a copy of the first element
        BT_RX_Buffer.erase(BT_RX_Buffer.begin());			// Remove the first element from the vector
        return String(DataElement.c_str());					// Return the copied data
	}
	else
	{
		return "";
	}
}



int BLESerial::Send_Data(String DataToSend)
{
	if(DeviceConnected)
	{
		pTxCharacteristic->setValue(DataToSend.c_str());
        pTxCharacteristic->notify();
		return 0;
	}
	else
	{
		return -1;
	}
}



void BLESerial::Deinit()
{
	// Reset the RX buffer
	BT_RX_Buffer.resize(0);

	// Stop the BLE Service
	pServer->getAdvertising()->stop();
	pServer->removeService(pServer->getServiceByUUID(UART_SERVICE_UUID));
	BLEDevice::deinit();

	// Reset pointers
	pServer = nullptr;
	pTxCharacteristic = nullptr;
}
