#include <Arduino.h> // Default Arduino library
#include <ESP32-TWAI-CAN.hpp> // ESP32-TWAI-CAN library on GitHub
#include <stdarg.h> // For using functions with variable parameters

#include "lorae5.h" // LoRaE5 library on GitHub
// Configuration for LoRaWAN
#include "config_application.h" // Configuration for LoRa module End Device
#include "config_board.h" // Standard configuration for 

/* Pins */
#define RX_PIN_CAN 22 // CAN RX Pin
#define TX_PIN_CAN 21 // CAN TX Pin

#define RX_PIN_LORA 16 // UART RX Pin
#define TX_PIN_LORA 17 // UART TX Pin

/* CAN */
CanFrame rxFrame; // CAN frame to read

// Request format: 
// Number of bytes that will follow
// Request mode (0x01, current data)
// PID
// Padding
//byte requestSupportedPIDs[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // 0x00=Availible PID's from 0x00-0x20
byte requestRPM[8] = { 0x02, 0x01, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00 }; //0x0C=Engine RPM
byte requestSpeed[8] = { 0x02, 0x01, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00 }; //0x0D=Vehicle Speed
byte requestMAF[8] = { 0x02, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00 }; //0x10=Mass AirFlow (MAF)
byte requestFuel[8] = { 0x02, 0x01, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x00 }; //0x2F=Fuel level Input
byte requestArray[8]; // Either one of the above

int CANFrameCount = 4; // Different PID requests count

struct OBD2_Data { // Format for decoded data
  int rpm;
  byte speed;
  float maf;
  float fuel;
};
OBD2_Data currentData; // To keep track of a single data point
OBD2_Data averageData; // To keep track of averages

/* LoRa */
LORAE5 lorae5(devEUI, appEUI, appKey, devAddr, nwkSKey, appSKey);
bool joined = false;

/* General */
typedef void (*DecodeFunc)(); // Function pointer for decode methods

bool startup = true; // Startup sending to LoRaWAN and debug for startup PID check
unsigned long lastSend = 0;
const unsigned long intervalMinutes = 30; // Send to LoRaWAN every X minutes

void setup() {
  Debug_Serial.begin(115200);
  
  // CAN setup
  ESP32Can.setPins(TX_PIN_CAN, RX_PIN_CAN);
  bool can = ESP32Can.begin(ESP32Can.convertSpeed(500)); // Start the CAN bus at 250 kbps or 500 kbps (faster)
  if(can)
    Serial.println("CAN bus started!");
  else
    Serial.println("CAN bus failed!");

  // LoRa setup
  LoRa_Serial.begin(9600, SERIAL_8N1, RX_PIN_LORA, TX_PIN_LORA); // Use serial for LoRa communication on defined pins
  lorae5.setup_hardware(&Debug_Serial, &LoRa_Serial);
  delay(1000);
  lorae5.setup_lorawan(REGION, ACTIVATION_MODE, CLASS, SPREADING_FACTOR, ADAPTIVE_DR, CONFIRMED, FPORT, SEND_BY_PUSH_BUTTON, FRAME_DELAY);
  lorae5.printInfo(); // Print LoRaE5 library setup info
  delay(1000);
  lorae5.sleep(); // Enter lowpower mode
  Debug_Serial.println("Entering low power mode!");
}

void loop() {
  CANState();

  if (millis() - lastSend >= (intervalMinutes * 60 * 1000 ) || startup) { // Send using LoRaWAN, convert 30 minutes to milliseconds
    lastSend = millis(); // Keep track of time
    LoRaConnectionState(averageData);
    startup = false;
  }
  
  waitingState();
}

void waitingState() {
  Debug_Serial.println("Sleeping for 3 seconds...");
  delay(3000); // 3 seconds
}

/* CAN-bus */
void CANState() {
  for (int i = 0; i < CANFrameCount; i++) { // Send all CAN requests one by one
    DecodeFunc decodeMethod = CANStatus(i); // Returns pointer to decode method
    if (decodeMethod != nullptr)
      decodeMethod(); // Decode the CAN data
  }
}

DecodeFunc CANStatus(int i){
  DecodeFunc decodeMethod = nullptr;
  switch (i) {
    case 0:
      Debug_Serial.println("Sending: RPM");
      for (int i = 0; i < sizeof(requestRPM); i++)
        requestArray[i] = requestRPM[i];
      decodeMethod = &decodeRPM;
    break;

    case 1:
      Debug_Serial.println("Sending: Vehicle Speed");
      for (int i = 0; i < sizeof(requestSpeed); i++) 
        requestArray[i] = requestSpeed[i];
      decodeMethod = &decodeSpeed;
    break;

    case 2:
      Debug_Serial.println("Sending: MAF");
      for (int i = 0; i < sizeof(requestMAF); i++) 
        requestArray[i] = requestMAF[i];
      decodeMethod = &decodeMAF;
    break;

    case 3:
      Debug_Serial.println("Sending: Fuel Level Input");
      for (int i = 0; i < sizeof(requestFuel); i++) 
        requestArray[i] = requestFuel[i];
      decodeMethod = &decodeFuel;
    break;
  }

  // Debug code for checking availible PID's
  // if (startup) { // Request availible PID'son startup
  //   for (int i = 0; i < sizeof(requestSupportedPIDs); i++)
  //     requestArray[i] = requestSupportedPIDs[i];
  // }
  sendCANDataState(requestArray);
  delay(1000); // Wait for timeout of request (1000 milliseconds) before receiving
  receiveCANDataState();
  return decodeMethod;
}

void sendCANDataState(byte array[8]) {
  Debug_Serial.print("CAN: Sending packet... ");

  CanFrame frame = {0};
  frame.identifier = 0x7DF; // 0x7DF = request to all ECUs
  frame.extd = 0; // Set extended frame to false (11-bit, 29-bit)
  frame.data_length_code = 8; // Bytes that will be sent
  for (int j = 0; j < 8; j++)
    frame.data[j] = array[j];
  
  ESP32Can.writeFrame(frame); // Send frame on CAN bus

  Debug_Serial.println("CAN frame sent!");
}

void receiveCANDataState() {
  if(ESP32Can.readFrame(rxFrame)) {
    Debug_Serial.printf("CAN: Received frame: %03X \r\n", rxFrame.identifier);

    // Print data bytes from the frame
    for(int i = 0; i <= rxFrame.data_length_code - 1; i ++)
      Debug_Serial.printf("%02X (%d) ", rxFrame.data[i], rxFrame.data[i]); // Hex and decimal print
    Debug_Serial.println();
  } else
    Debug_Serial.println("No frame recieved");
}

/* LoRaWAN */
void LoRaConnectionState(OBD2_Data averageData){
  // Connect to LoRaWAN mesh network
  if(ACTIVATION_MODE == OTAA) { // Join network using OTAA
    LoRa_Serial.println("AT"); // Exit lowpower mode
    Debug_Serial.println("Exiting low power mode!");
    delay(1000); // Wait for module to wake up

    Debug_Serial.println("Checking joined status...");
    LoRa_Serial.println("AT+JOIN"); // AT command
    delay(500); // Wait for response
    String response = "";
    while (LoRa_Serial.available()) // Read response
      response += (char)LoRa_Serial.read();
    Debug_Serial.println(response);

    if (response.indexOf("+JOIN: Joined already") != -1) { // Still joined because status exists
      Debug_Serial.println("Still joined to the network! Sending data...");
      LoRaSendState(averageData);
    } else { // Not joined, retry using library
      Debug_Serial.println("Not joined to the network! Retrying connection...");

      for (int joinAttempts = 1; joinAttempts <= 5; joinAttempts++) { // Attempt to join 3 times
        Debug_Serial.printf("Join attempt: %d\n", joinAttempts);
        joined = lorae5.join(); // Try joining the network
        if (joined) 
          break; // Stop attempting to join
      }

      if (joined) {
        Debug_Serial.println("Rejoined to the network! Sending data...");
        LoRaSendState(averageData);
      } else // No connection in range
        Debug_Serial.println("Still not joined to the network! Retrying next time.");
    }
  }
}

void LoRaSendState(OBD2_Data averageData){
  uint8_t payloadUp[4]; // 4 Bytes
  uint8_t sizePayloadUp = sizeof(payloadUp);

  // Fill payload bytes with average data of period (30 mins)
  uint8_t rpm = averageData.rpm / 100; // Max rpm=10000, count in hundreds to fit in one byte so /100
  uint8_t speed = averageData.speed; // Max speed=255, fits in one byte
  // Convert from float to int to remove decimals so that they each fit in one byte
  uint8_t maf = (uint8_t)averageData.maf; 
  uint8_t fuel = (uint8_t)averageData.fuel;

  payloadUp[0] = rpm;
  payloadUp[1] = speed;
  payloadUp[2] = maf;
  payloadUp[3] = fuel;

  // Debug print for payload hexdata
  // Debug_Serial.println("Payload: ");
  // for (int i = 0; i < sizePayloadUp; i++)
  //   Debug_Serial.print(payloadUp[i]);
  // Debug_Serial.println();

  lorae5.sendData(payloadUp, sizePayloadUp);
  lorae5.sleep(); // Enter lowpower mode
  Debug_Serial.println("Entering low power mode!");

  // Reset averages that were tracked before this period
  averageData.rpm = 0;
  averageData.speed = 0;
  averageData.maf = 0;
  averageData.fuel = 0;
}

/* Decodes */
void decodeRPM() {
  int A = rxFrame.data[3];
  int B = rxFrame.data[4];
  currentData.rpm = ((256*A) + B) / 4; // Combine 2 bytes into 1 and divide by 4
  //currentData.rpm = 100; // Debug
  Debug_Serial.printf("RPM: %d\n", currentData.rpm);

  if (averageData.rpm == 0) // First data
    averageData.rpm = currentData.rpm;
  else 
    averageData.rpm = (averageData.rpm + currentData.rpm) / 2;
  Debug_Serial.printf("Average Rounds Per Minute: %d RPM\n", averageData.rpm);
}

void decodeSpeed() {
  currentData.speed = rxFrame.data[3]; // Direct, 0-255 km/h
  //currentData.speed = 20; // Debug
  Debug_Serial.printf("Speed: %d km/h\n", currentData.speed);
  
  if (averageData.speed == 0) // First data
    averageData.speed = currentData.speed;
  else 
    averageData.speed = (averageData.speed + currentData.speed) / 2;
  Debug_Serial.printf("Average Vehicle Speed: %d km/h\n", averageData.speed);
}

void decodeMAF() {
  int A = rxFrame.data[3];
  int B = rxFrame.data[4];
  currentData.maf = ((256*A) + B) / 100; // Combine 2 bytes into 1 and divide by 100
  //currentData.maf = 1; // Debug
  Debug_Serial.printf("MAF: %.2f g/s\n", currentData.maf); // Print as float (2 decimals)

  if (averageData.maf == 0) // First data
    averageData.maf = currentData.maf;
  else 
    averageData.maf = (averageData.maf + currentData.maf) / 2;
  Debug_Serial.printf("Average Mass AirFlow: %.2f g/s\n", averageData.maf);
}

void decodeFuel() {
  byte fuelLevelHex = rxFrame.data[3];
  currentData.fuel = fuelLevelHex * 100.0 / 255.0; // Convert byte 0-255 to percentage 0-100%
  //currentData.fuel = 25; // Debug
  Debug_Serial.printf("Fuel: %.1f %%\n", currentData.fuel); // Print as float (1 decimal) in percentage
  
  if (averageData.fuel == 0) // First data
    averageData.fuel = currentData.fuel;
  else 
    averageData.fuel = (averageData.fuel + currentData.fuel) / 2;
  Debug_Serial.printf("Average Fuel Level Input: %.1f %%\n", averageData.fuel);
}