/***********************************************************************/
/* Please see README page on https://github.com/SylvainMontagny/LoRaE5 */
/***********************************************************************/

#define REGION EU868 // EU868=frequency
#define ACTIVATION_MODE OTAA
// Class A=for low-power devices (low-power mode)
//See https://www.rfwireless-world.com/tutorials/understanding-lora-lorawan-class-a-b-c 
#define CLASS CLASS_A 
#define SPREADING_FACTOR 8
#define ADAPTIVE_DR false // False=mobile device, see https://www.thethingsnetwork.org/docs/lorawan/adaptive-data-rate/ 
#define CONFIRMED false
#define FPORT 1 // FPort 1-223 = application specific data

#define SEND_BY_PUSH_BUTTON false
// 6000 for SF12 (max range), 4000 for SF11, 3000 for SF11, 2000 for SF9/8/, 1500 fo  r SF7, 
//see https://github.com/andresoliva/LoRa-E5/blob/main/examples/Grove-Wio-E5_basic/Grove-Wio-E5_basic.ino 
#define FRAME_DELAY 20000 

String devEUI = "";

// Configuration for ABP Activation Mode
String devAddr = "00000000";
String nwkSKey = "00000000000000000000000000000000";
String appSKey = "00000000000000000000000000000000";

// Configuration for OTAA Activation Mode
String appKey = ""; 
String appEUI = "8000000000000006";
