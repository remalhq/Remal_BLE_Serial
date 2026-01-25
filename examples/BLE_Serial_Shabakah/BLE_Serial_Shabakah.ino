/**
 * @file    BLE_Serial_Shabakah.ino
 * @authors Khalid Mansoor AlAwadhi, Remal <khalid@remal.io>
 * @date    25 Jan 2025
 *
 * @brief   This example shows how to use the Remal BLE Serial library with the Shabakah board's
 *          onboard RGB LEDs to provide visual feedback for BLE connection and data status.
 *
 *          This example will:
 *            - Display any data received from the connected Bluetooth device on the Serial Monitor
 *            - Every 1 second, a message will be sent to the connected device
 *            - LED 1 on Shabakah will be RED when not connected, GREEN when connected
 *            - LED 2 on Shabakah will flash BLUE when receiving data from a connected device
 *
 *          To send or receive data from the Shabakah board, you can use one of the following apps:
 *            - (iOS - Paid) BLE Terminal HM-10: https://apps.apple.com/us/app/ble-terminal-hm-10/id1398703795
 *            - (iOS - Free) Bluefruit Connect: https://apps.apple.com/us/app/bluefruit-connect/id830125974
 *            - (Android - Free) Serial Bluetooth Terminal: https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal&hl=en&gl=US
 *
 *          Make sure you select "Shabakah v4" under "Tools -> Boards -> Remal Boards (ESP32)"
 *          and the correct port under "Tools -> Port"
 */
#include "Remal_BLE_Serial.h"
#include "Adafruit_NeoPixel.h"



/*#############################################
 * Shabakah Board Defines and Global Variables
 *#############################################*/
/* RGB LED (WS2812B) */
const int LED_1_PIN = 1;                    //The pin connected to RGB LED 1 on Shabakah
const int LED_2_PIN = 3;                    //The pin connected to RGB LED 2 on Shabakah
const int NumLEDs = 1;                      //Number of LEDs on each pin
Adafruit_NeoPixel Shabakah_LED_1(NumLEDs, LED_1_PIN, NEO_GRB + NEO_KHZ800);       //Object to control LED 1
Adafruit_NeoPixel Shabakah_LED_2(NumLEDs, LED_2_PIN, NEO_GRB + NEO_KHZ800);       //Object to control LED 2

/* Bluetooth */
BLESerial BT_Device;

/* Buffers */
char RX_Buffer[128];                        //Buffer for received data

/* Connection state */
bool WasConnected = false;                  //Tracks previous connection state

/* Loop timing counters */
int Counter_HelloMsg = 0;                   //Used to send messages at a predetermined interval



void setup()
{
  /* Initialize the Serial Monitor */
  Serial.begin(115200);

  /* Initialize the RGB LEDs to be off */
  Shabakah_LED_1.begin();
  Shabakah_LED_1.setBrightness(50);
  Shabakah_LED_1.clear();
  Shabakah_LED_1.show();

  Shabakah_LED_2.begin();
  Shabakah_LED_2.setBrightness(200);
  Shabakah_LED_2.clear();
  Shabakah_LED_2.show();

  /* Initialize the Bluetooth */
  BT_Device.Init("Shabakah");

  Serial.println("> BLE Serial Shabakah Example");
  Serial.println("> LED 1: RED = not connected, GREEN = connected");
  Serial.println("> LED 2: BLUE flash = receiving data");
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

  /* Update LED 1 based on connection state */
  if(IsConnected)
  {
    /* If connected, turn LED 1 GREEN */
    Shabakah_LED_1.setPixelColor(0, Shabakah_LED_1.Color(0, 255, 0));
    Shabakah_LED_1.show();

    /* Check if we have any data available to read */
    while( BT_Device.Data_Available() > 0 )
    {
      /* While getting data, turn LED 2 BLUE */
      Shabakah_LED_2.setPixelColor(0, Shabakah_LED_2.Color(0, 0, 255));
      Shabakah_LED_2.show();

      /* Get the data into our buffer */
      int Len = BT_Device.Get_Data(RX_Buffer, sizeof(RX_Buffer));
      if(Len > 0)
      {
        Serial.print("> Received: ");
        Serial.println(RX_Buffer);
      }

      /* Clear LED 2 */
      Shabakah_LED_2.clear();
      Shabakah_LED_2.show();
    }

    /* Send a message every 1 second */
    if(Counter_HelloMsg >= 1000)
    {
      BT_Device.Send_Data("Hello from Shabakah!\r\n");
      Serial.println("> Sent: Hello from Shabakah!");
      Counter_HelloMsg = 0;
    }
  }
  else
  {
    /* If not connected, turn LED 1 RED */
    Shabakah_LED_1.setPixelColor(0, Shabakah_LED_1.Color(255, 0, 0));
    Shabakah_LED_1.show();
  }

  Counter_HelloMsg++;
  delay(1);
}
