// Debug for Dynamic Networking Module
//
// Overrides "Dynamic Networking Module" macros to test out the functionality
// without changing the source code. Requires "Dynamic Netowrking Module"
// (network.h).
//
// Only works if DEBUG_MODE is set to 1 before importing this header.
//
// Sets all the mandatory network.h config macros.
//
// Regardless of NETWORK_MODE will define the following for the user:
// - DEBUG_NETWORK_STREAM: Serial stream used to send the SPI output to
//   (defaults to `Serial`).
// - DEBUG_BAUD_RATE: The baudrate used for the DEBUG_NETWORK_STREAM (defaults
//   to 9600).

#ifndef DEBUG_NETWORK_H_
#define DEBUG_NETWORK_H_

// Defaults
#ifndef DEBUG_NETWORK_MODE
#define DEBUG_NETWORK_MODE 0
#endif // DEBUG_NETWORK_MODE

#ifndef DEBUG_NETWORK_STREAM
#define DEBUG_NETWORK_STREAM Serial
#endif // DEBUG_NETWORK_STREAM

#ifndef DEBUG_BAUD_RATE
#define DEBUG_BAUD_RATE 9600
#endif // DEBUG_BAUD_RATE

#if DEBUG_MODE == 1

#define NETWORK_MODE -1
#define SSID "debug"
#define WIFI_PASSWORD "debug"

#define NETWORK_CLIENT typeof(DEBUG_NETWORK_STREAM)
#define NETWORK_INIT(variable_name)                                            \
  NETWORK_CLIENT *variable_name = &DEBUG_NETWORK_STREAM
#define NETWORK_SETUP()                                                        \
  DEBUG_NETWORK_STREAM.begin(DEBUG_BAUD_RATE);                                 \
  _mac2str(_macstr, _macarr)
#define NETWORK_LOOP()
#define NETWORK_CONNECT(client, ...) true
#define NETWORK_CONNECTED(client) false
#define NETWORK_STOP(client) true
#define MAC String(_macstr)

#endif // DEBUG_MODE

#endif // DEBUG_NETWORK_H_
