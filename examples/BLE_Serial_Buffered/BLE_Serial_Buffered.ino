/**
 * @file    BLE_Serial_Buffered.ino
 * @authors Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * @date    25 Jan 2025
 *
 * @brief   This example demonstrates the buffered transmission feature of the Remal BLE Serial library.
 *          Buffering allows you to accumulate multiple Send_Data() calls into a single BLE notification,
 *          which is useful when building messages from multiple parts (e.g., logging with timestamps,
 *          levels, and formatted output).
 *
 *          This example will:
 *            - Initialize BLE with the device name "Remal Board"
 *            - Every 2 seconds, send a buffered log message that is built from multiple parts
 *            - Without buffering, each part would be sent as a separate BLE notification
 *            - With buffering, all parts are combined and sent as ONE notification
 *
 *          To send or receive data from the board, you can use one of the following apps:
 *            - (iOS - Paid) BLE Terminal HM-10: https://apps.apple.com/us/app/ble-terminal-hm-10/id1398703795
 *            - (iOS - Free) Bluefruit Connect: https://apps.apple.com/us/app/bluefruit-connect/id830125974
 *            - (Android - Free) Serial Bluetooth Terminal: https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US
 *
 *          Make sure you select "Shabakah v4" under "Tools -> Boards -> Remal Boards (ESP32)"
 *          and the correct port under "Tools -> Port"
 */
#include "Remal_BLE_Serial.h"



/*#############################################
 * Defines and Global Variables
 *#############################################*/
/* Bluetooth */
BLESerial BT_Device;

/* Buffers */
char RX_Buffer[128];                        //Buffer for received data
char TX_Buffer[64];                         //Buffer for formatted messages

/* Connection state */
bool WasConnected = false;                  //Tracks previous connection state

/* Loop timing counters */
int Counter_LogMsg = 0;                     //Used to send log messages at a predetermined interval
int Log_Number = 0;                         //Tracks the number of log messages sent



/*#############################################
 * Helper Functions
 *#############################################*/
/**
 * @brief   Sends a formatted log message using buffered transmission
 *          The message is built from multiple parts but sent as ONE BLE notification
 */
void SendLogMessage(const char* Level, const char* Source, const char* Message)
{
  BT_Device.Begin_Buffer();           // Start accumulating data

  BT_Device.Send_Data("[");
  BT_Device.Send_Data(Level);
  BT_Device.Send_Data("] ");
  BT_Device.Send_Data(Source);
  BT_Device.Send_Data(": ");
  BT_Device.Send_Data(Message);
  BT_Device.Send_Data("\r\n");

  BT_Device.Flush_Buffer();           // Send everything as ONE notification
}



void setup()
{
  /* Initialize the Serial Monitor */
  Serial.begin(115200);

  /* Initialize the Bluetooth */
  BT_Device.Init("Remal Board");

  Serial.println("> BLE Serial Buffered Example");
  Serial.println("> Device name: Remal Board");
  Serial.println("> Waiting for connection...");
}


void loop()
{
  /* Check for connection state changes */
  bool IsConnected = BT_Device.IsConnected();
  if(IsConnected != WasConnected)
  {
    WasConnected = IsConnected;
    if(IsConnected)
    {
      Serial.println("> Device connected!");
    }
    else
    {
      Serial.println("> Device disconnected!");
    }
  }

  /* Only process data if connected */
  if(IsConnected)
  {
    /* Check if we have any data available to read */
    while( BT_Device.Data_Available() > 0 )
    {
      /* Get the data into our buffer */
      int Len = BT_Device.Get_Data(RX_Buffer, sizeof(RX_Buffer));
      if(Len > 0)
      {
        Serial.print("> Received: ");
        Serial.println(RX_Buffer);

        /* Send a buffered response */
        SendLogMessage("DEBUG", "RX", RX_Buffer);
      }
    }

    /* Send a buffered log message every 2 seconds */
    if(Counter_LogMsg >= 2000)
    {
      Log_Number++;
      snprintf(TX_Buffer, sizeof(TX_Buffer), "Log entry #%d", Log_Number);
      SendLogMessage("INFO", "Loop", TX_Buffer);
      Serial.print("> Sent buffered log #");
      Serial.println(Log_Number);
      Counter_LogMsg = 0;
    }
  }

  Counter_LogMsg++;
  delay(1);
}
