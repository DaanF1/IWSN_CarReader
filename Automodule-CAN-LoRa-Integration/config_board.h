#ifndef CONFIG_BOARD_H
#define CONFIG_BOARD_H

//////////////////////////////////////////////
// If you use an ARDUINO or ESP32 board 
//////////////////////////////////////////////
  #if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAM) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_WIO_TERMINAL)
    #define Debug_Serial      Serial    // Select the right SerialX for Debug

    // Arduino's Serial Documentation : https://www.arduino.cc/reference/en/language/functions/communication/serial/
    // ESP32 Documentation : https://microcontrollerslab.com/esp32-uart-communication-pins-example/
    #define LoRa_Serial       Serial2   // Select the right SerialX for the LoRaE5 connection

//////////////////////////////////////////////
// /!\ Your card has never been tested yet !
// Please let us know if you find the PIN configuration of your board
// So we can add it to the next version of this code
//////////////////////////////////////////////

  #else
  #warning "Your board has never been tested with this lib"
  #warning "Select the right Serial for Debug and LoRaE5 connection in config_board.h"

  #define Debug_Serial      Serial
  #define LoRa_Serial       Serial1

  #endif

#endif //CONFIG_BOARD_H