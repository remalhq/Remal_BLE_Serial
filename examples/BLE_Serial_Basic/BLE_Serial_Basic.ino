/**
 * @file    BLE_Serial_Basic.ino
 * @authors Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * @date    25 Jan 2025
 *
 * @brief   This example shows how to use the Remal BLE (Bluetooth Low Energy) Serial library to send and receive
 *          data over Bluetooth LE. It works with any ESP32 based Remal board.
 *
 *          This example will:
 *            - Initialize BLE with the device name "Remal Board"
 *            - Display any data received from the connected Bluetooth device on the Serial Monitor
 *            - Echo back the received data to the connected device
 *            - Every 2 seconds, send a "Hello" message to the connected device
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
char TX_Buffer[128];                        //Buffer for data to send

/* Connection state */
bool WasConnected = false;                  //Tracks previous connection state

/* Loop timing counters */
int Counter_SendMsg = 0;                    //Used to send messages at a predetermined interval
int Msg_Number = 0;                         //Tracks the number of messages sent



void setup()
{
  /* Initialize the Serial Monitor */
  Serial.begin(115200);

  /* Initialize the Bluetooth */
  BT_Device.Init("Remal Board");

  Serial.println("> BLE Serial Basic Example");
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

        /* Echo the data back */
        snprintf(TX_Buffer, sizeof(TX_Buffer), "Echo: %s\r\n", RX_Buffer);
        BT_Device.Send_Data(TX_Buffer);
      }
    }

    /* Send a message every 2 seconds */
    if(Counter_SendMsg >= 2000)
    {
      Msg_Number++;
      snprintf(TX_Buffer, sizeof(TX_Buffer), "Hello from Remal Board #%d\r\n", Msg_Number);
      BT_Device.Send_Data(TX_Buffer);
      Serial.print("> Sent: ");
      Serial.print(TX_Buffer);
      Counter_SendMsg = 0;
    }
  }

  Counter_SendMsg++;
  delay(1);
}
