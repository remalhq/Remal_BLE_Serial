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
static size_t BT_RX_BufferMaxEntries = 128;		// Max queued RX messages
static SemaphoreHandle_t BT_RX_BufferMutex = NULL;	// Protect RX queue shared by BLE callback/tasks


/***************************************************************
 * 				Private callback functions
 ***************************************************************/
/*
 * @brief   This callback is called when a device connects or disconnects
 */
class Server_Callbacks: public NimBLEServerCallbacks
{
	void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override
	{
		DeviceConnected = true;
	}

	void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override
	{
		DeviceConnected = false;

		/* Restart advertising */
		delay(500);
		NimBLEDevice::getAdvertising()->start();
	}
};

/*
 * @brief   This callback is called when the RX characteristic is written to
 */
class RX_Callbacks: public NimBLECharacteristicCallbacks
{
	void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override
	{
		std::string RX_Value = pCharacteristic->getValue();

		/* Store the received data in the RX buffer */
		if (!RX_Value.empty() && BT_RX_BufferMutex != NULL)
		{
			if (xSemaphoreTake(BT_RX_BufferMutex, 0) == pdTRUE)
			{
				if (BT_RX_BufferMaxEntries > 0)
				{
					while (BT_RX_Buffer.size() >= BT_RX_BufferMaxEntries)
					{
						BT_RX_Buffer.erase(BT_RX_Buffer.begin());
					}

					BT_RX_Buffer.push_back(RX_Value);
				}

				xSemaphoreGive(BT_RX_BufferMutex);
			}
		}
	}
};



/***************************************************************
 * 				Class Functions
 ***************************************************************/
void BLESerial::Init(const char* DeviceName)
{
	NimBLEDevice::init(DeviceName);							// Initialize BLE device

	// Optional: Set preferred MTU for larger payloads
	NimBLEDevice::setMTU(517);

	pServer = NimBLEDevice::createServer();					// Create the BLE Server
	pServer->setCallbacks(new Server_Callbacks());			// Set the server callbacks

	// Create the BLE Service (UART)
	NimBLEService *pService = pServer->createService(UART_SERVICE_UUID);

	// Create BLE TX and RX Characteristics (the TX/RX namings are from the client's perspective)
	// NimBLE automatically creates the 2902 descriptor for NOTIFY characteristics
	pTxCharacteristic = pService->createCharacteristic(
		UART_CHARACTERISTIC_UUID_RX,
		NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY
	);

	NimBLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
		UART_CHARACTERISTIC_UUID_TX,
		NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR
	);
	pRxCharacteristic->setCallbacks(new RX_Callbacks());	// Set the RX callback

	if (BT_RX_BufferMutex == NULL)
	{
		BT_RX_BufferMutex = xSemaphoreCreateMutex();
	}

	// Initialize RX queue
	if (BT_RX_BufferMutex != NULL)
	{
		xSemaphoreTake(BT_RX_BufferMutex, portMAX_DELAY);
		BT_RX_Buffer.reserve(BT_RX_BufferMaxEntries);
		BT_RX_Buffer.clear();
		xSemaphoreGive(BT_RX_BufferMutex);
	}

	// Initialize TX buffer
	TX_Buffer.clear();
	BufferingEnabled = false;
	TX_BufferMaxSize = 512;

	// Start the service
	pService->start();

	// Configure advertising so the UART service UUID is advertised and the
	// full device name is returned in the scan response.
	NimBLEAdvertising *pAdvertising = NimBLEDevice::getAdvertising();
	pAdvertising->enableScanResponse(true);
	pAdvertising->addServiceUUID(UART_SERVICE_UUID);	// Required for iOS to discover device
	pAdvertising->setName(DeviceName);
	pAdvertising->start();
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

	BT_RX_BufferMaxEntries = (size_t)Size;

	if (BT_RX_BufferMutex != NULL)
	{
		xSemaphoreTake(BT_RX_BufferMutex, portMAX_DELAY);

		while (BT_RX_Buffer.size() > BT_RX_BufferMaxEntries)
		{
			BT_RX_Buffer.erase(BT_RX_Buffer.begin());
		}

		BT_RX_Buffer.reserve(BT_RX_BufferMaxEntries);
		xSemaphoreGive(BT_RX_BufferMutex);
	}
}



int BLESerial::Data_Available()
{
	int AvailableCount = 0;

	if (BT_RX_BufferMutex != NULL)
	{
		xSemaphoreTake(BT_RX_BufferMutex, portMAX_DELAY);
		AvailableCount = (int)BT_RX_Buffer.size();
		xSemaphoreGive(BT_RX_BufferMutex);
	}

	if (AvailableCount > 0)
	{
		return AvailableCount;
	}

	if (DeviceConnected)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}



int BLESerial::Get_Data(char* Buffer, size_t BufferSize)
{
	/* Check parameters */
	if(Buffer == nullptr || BufferSize == 0)
	{
		return -1;
	}

	if (BT_RX_BufferMutex == NULL)
	{
		Buffer[0] = '\0';
		return 0;
	}

	/* Check if we have any data available to read */
	xSemaphoreTake(BT_RX_BufferMutex, portMAX_DELAY);

	if( !BT_RX_Buffer.empty() )
	{
		std::string DataElement = BT_RX_Buffer.front();		// Get a copy of the first element
		BT_RX_Buffer.erase(BT_RX_Buffer.begin());			// Remove the first element from the vector
		xSemaphoreGive(BT_RX_BufferMutex);

		/* Copy to user buffer (with null termination) */
		size_t CopyLen = DataElement.length();
		if(CopyLen >= BufferSize)
		{
			CopyLen = BufferSize - 1;						// Leave room for null terminator
		}
		memcpy(Buffer, DataElement.c_str(), CopyLen);
		Buffer[CopyLen] = '\0';								// Null terminate

		return (int)CopyLen;
	}
	else
	{
		xSemaphoreGive(BT_RX_BufferMutex);
		Buffer[0] = '\0';									// Empty string
		return 0;
	}
}



int BLESerial::Send_Data(const char* DataToSend)
{
	if(!DeviceConnected)
	{
		return -1;
	}

	if(DataToSend == nullptr)
	{
		return -1;
	}

	size_t DataLen = strlen(DataToSend);

	if(BufferingEnabled)
	{
		// Append to buffer (with overflow protection)
		size_t newLen = TX_Buffer.length() + DataLen;
		if(newLen <= TX_BufferMaxSize)
		{
			TX_Buffer += DataToSend;
		}
		// Data buffered, will be sent on Flush_Buffer()
		return 0;
	}
	else
	{
		// Immediate send (original behavior)
		pTxCharacteristic->setValue((uint8_t*)DataToSend, DataLen);
		pTxCharacteristic->notify();
		return 0;
	}
}



void BLESerial::Deinit()
{
	// Reset the RX buffer
	if (BT_RX_BufferMutex != NULL)
	{
		xSemaphoreTake(BT_RX_BufferMutex, portMAX_DELAY);
		BT_RX_Buffer.clear();
		xSemaphoreGive(BT_RX_BufferMutex);
	}

	// Reset TX buffer
	TX_Buffer.clear();
	BufferingEnabled = false;

	// Stop advertising and deinit
	NimBLEDevice::stopAdvertising();
	NimBLEDevice::deinit(true);		// true = release memory

	// Reset pointers
	pServer = nullptr;
	pTxCharacteristic = nullptr;
}



void BLESerial::Begin_Buffer()
{
	BufferingEnabled = true;
	TX_Buffer.clear();
}



int BLESerial::Flush_Buffer()
{
	if(!DeviceConnected)
	{
		BufferingEnabled = false;
		TX_Buffer.clear();
		return -1;
	}

	if(TX_Buffer.empty())
	{
		BufferingEnabled = false;
		return -2;		// Nothing to send
	}

	// Send buffered data as single notification
	pTxCharacteristic->setValue((uint8_t*)TX_Buffer.c_str(), TX_Buffer.length());
	pTxCharacteristic->notify();

	// Reset state
	TX_Buffer.clear();
	BufferingEnabled = false;

	return 0;
}



bool BLESerial::Is_Buffering()
{
	return BufferingEnabled;
}



void BLESerial::Clear_Buffer()
{
	TX_Buffer.clear();
	BufferingEnabled = false;
}



void BLESerial::Set_TX_BufferSize(size_t Size)
{
	if(Size > 0)
	{
		TX_BufferMaxSize = Size;
	}
}



size_t BLESerial::Get_BufferLength()
{
	return TX_Buffer.length();
}
