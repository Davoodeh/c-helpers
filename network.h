// Dynamic Networking Module
//
// Define macroes below before importing the header to make use of dynamic
// networking (if NETWORK_MODE not define will default to Ethernet and if
// NETWORK_MODE is defined but not the NETWORK_PASSWORD or NETWORK_SSID will
// lead to unpredictable behavior):
// ```c
// #define NETWORK_MODE 0 // 0 for Ethernet and 1 for Wifi (default ethernet)
// #define NETWORK_SSID "ssid"              // MANDATORY when on WIFI
// #define NETWORK_PASSWORD "12345678"      // MANDATORY when on WIFI
// #define NETWORK_IP  { 192, 168, 1, 155 } // optional
// #define NETWORK_MAC { 0xDE, 0xAD, 0xDE, 0xAD, 0xBE, 0xEF } // optional
// ```
//
// Conditionally includes <Wifi.h> if NETWORK_MODE is 1 otherwise includes
// "Ethernet.h" and <SPI.h> for ethernet.
//
// Optionally dependent on a macro name DBG which is equal to Serial.print()
// (applied conditionally).
//
// Regardless of NETWORK_MODE will define the following for the user (note that
// for macro functions, the signitures are symmetrical between both variants):
// - NETWORK_MAC: The network MAC String taken from the hardware (overrides the
//   config).
// - NETWORK_IP: The network IP taken from the hardware (overrides the config).
// - NETWORK_CLIENT: Object of network client used to set up the network
// - NETWORK_INIT(variable_name): Initialize network objects
// - NETWORK_SETUP(): Setups the network and connects to it (sets NETWORK_MAC
//   will be valid after calling this macro)
// - NETWORK_LOOP(): Ensures the connection to the network
// - NETWORK_CONNECT(client, ...): Same as client.connect.
// - NETWORK_CONNECTED(client): Same as client.connected.
// - NETWORK_STOP(client): Same as client.stop.
//
// Example:
// ```c
// // network.h configs
// // If true, the details of connecting is printed to Serial
// #define DEBUG_MODE 1
//
// #define NETWORK_MODE 0 // This time around, this program uses Ethernet
// #if NETWORK_MODE == 1  // WiFi specific settings
// #define NETWORK_SSID "myssid"
// #define NETWORK_PASSWORD "12345678"
// #endif // NETWORK_MODE
// // end network.h configs
//
// // Configs must come before the header inclusion
// #include "network.h"
//
// NETWORK_INIT(network);
//
// void setup() {
//   Serial.begin(9600);
//   NETWORK_SETUP();
// }
//
// void loop() {
//   NETWORK_LOOP(); // ensures connection
//   // At this point in the program, `network` is a ready to use
//   // ConnectionClient
//   delay(3000);
// }
// ```

#ifndef NETWORK_H_
#define NETWORK_H_

// Defaults
// Default mode (since Ethernet doesn't need much of a config)
#ifndef NETWORK_MODE
#define NETWORK_MODE 0
#endif // NETWORK_MODE

// Default ip for ethernet
#ifndef NETWORK_IP
#define NETWORK_IP                                                             \
  { 192, 168, 1, 155 }
#endif // NETWORK_IP

// Default mac for ethernet
#ifndef NETWORK_MAC
#define NETWORK_MAC                                                            \
  { 0xDE, 0xAD, 0xDE, 0xAD, 0xBE, 0xEF }
#endif // NETWORK_MAC

// Dependecies
// Make DBG macro optional
#ifndef DBG
#define DBG(...)
#endif // DBG

// Helper functions
// Defined regardless to be used in other headers
char *_macstr = (char *)malloc(13);
char *_mac2str(char *buf, byte macarr[]) {
  for (int i = 0; i < 6; i++) {
    char hex[3];
    sprintf(hex, "%x", macarr[i]);
    buf[i * 2] = hex[0];
    buf[i * 2 + 1] = hex[1];
  }
  buf[12] = '\0';
  return buf;
}

// Program
#if NETWORK_MODE == 0 // Ethernet

#include "Ethernet.h"
#include <SPI.h>
const byte _macarr[] = NETWORK_MAC;
const byte _iparr[] = NETWORK_IP;
IPAddress _ip(_iparr[0], _iparr[1], _iparr[2], _iparr[3]);
#define NETWORK_CLIENT EthernetClient
#define NETWORK_MAC String(_macstr)
#define NETWORK_IP Ethernet.localIP()
#define NETWORK_SETUP()                                                        \
  delay(1000);                                                                 \
  DBG("Initializing Ethernet...\n");                                           \
  if (Ethernet.begin(_macarr) == 0)                                            \
    Ethernet.begin(_macarr, _ip);                                              \
  DBG("IP: ");                                                                 \
  DBG(NETWORK_IP);                                                             \
  DBG("\n");                                                                   \
  _mac2str(_macstr, _macarr)
#define NETWORK_LOOP()

#elif NETWORK_MODE == 1 // WIFI

#include <WiFi.h>
#define NETWORK_CLIENT WiFiClient
#define NETWORK_MAC String(WiFi.macAddress())
#define NETWORK_IP WiFi.localIP()
#define NETWORK_SETUP()                                                        \
  WiFi.begin(NETWORK_SSID, NETWORK_PASSWORD);                                  \
  while (WiFi.status() != WL_CONNECTED) {                                      \
    delay(500);                                                                \
    DBG("Connecting to WiFi...\n");                                            \
  }                                                                            \
  DBG("Connected to the WiFi network\n");                                      \
  DBG("IP: ");                                                                 \
  DBG(NETWORK_IP);                                                             \
  DBG("\n")
#define NETWORK_LOOP()                                                         \
  if (WiFi.status() != WL_CONNECTED) {                                         \
    DBG("Disconnected Wifi... Trying to reconnect...\n");                      \
    NETWORK_SETUP();                                                           \
  }

#endif // NETWORK_MODE

#define NETWORK_CONNECT(client, ...) client.connect(__VA_ARGS__)
#define NETWORK_CONNECTED(client) client.connected()
#define NETWORK_INIT(variable_name) NETWORK_CLIENT variable_name
#define NETWORK_STOP(client, ...) client.stop()

#endif // NETWORK_H_
