// Dynamic Request Module
//
// Author: M. Yas. Davoodeh <MYDavoodeh@gmail.com>
//
// Define macroes below before importing the header to make use of dynamic
// requesting:
// ```c
// #define REQUEST_MODE 0        // 1 for MQTT and 0 for HTTP (default HTTP)
// #define REQUEST_URL "its.tue" // MANDATORY
// #define REQUEST_PATH /* MANDATORY (NOTE not to include the initial "/") */  \
//   "path_of_host_or_topic_of_mqtt"
// #define REQUEST_PORT 80   // optional
// #define REQUEST_CLIENT_ID // optional, used in MQTT
//                           // (default: NETWORK_MAC.c_str())
// #define REQUEST_USERNAME  // MANDATORY when on MQTT
// #define REQUEST_PASSWORD  // MANDATORY when on MQTT
// #define REQUEST_METHOD "POST"  // optional, used in HTTP, defaults to "GET"
//                                // (use all caps)
// #define REQUEST_REPLY_WAIT 100 // optional, if defined, will wait a few ms
//                                // before reading the network available
//                                // input (default 100)
//
// // optional headers used in HTTP, default: ""
// // NOTE don't leave the trailing \n
// // Also keep in mind that the function used to make http_requests have some
// // default headers. Read the docstring of it for more details
// #define REQUEST_HEADERS "Authorization: bear\nContent-Type: application/json"
// ```
//
// Includes "PubSubClient.h" on MQTT mode.
//
// Depends on one other module "Dynamic Networking Module" (network.h) and must
// be imported after it.
//
// Optionally dependent on a macro name DBG which is equal to Serial.print()
// (applied conditionally).
//
// Regardless of REQUEST_MODE config will define the following for the user
// (note that for macro functions, the signitures are symmetrical between both
// variants):
// - REQUEST_INIT(net_client, variable_name): Initialize the transfer protocol
//   objects using a previously initialized net_client of type NETWORK_CLIENT.
// - REQUEST_SETUP(client): Set ups the transfer protocol client and connects.
// - REQUEST_LOOP(client): Ensures the necessary functionalities for the
//   protocol to continue to work.
// - REQUEST_SEND(client, data): The main function for sending over data to the
//   protocol based on the default config.
//
// Only in HTTP mode:
// - http_request(...): See the docstring
//
// Example:
// ```c
//
// // network.h and request.h common configs
// // If true, the details of connecting is printed to Serial
// #define DEBUG_MODE 1
//
// * // network.h configs
// #define NETWORK_MODE 0 // This time around, this program uses Ethernet
// #if NETWORK_MODE == 1  // WiFi specific settings
// #define NETWORK_SSID "myssid"
// #define NETWORK_PASSWORD "12345678"
// #endif // NETWORK_MODE
// // end network.h configs
//
//
// // request.h config
// #define REQUEST_MODE 0 // This time around, this program uses HTTP
// #if REQUEST_MODE == 0  // HTTP specific settings
// #define REQUEST_URL "httpbin.org"
// #define REQUEST_PATH "post"
// #define REQUEST_METHOD "POST"
// #define REQUEST_REPLY_WAIT 3000
// #elif REQUEST_MODE == 1 // MQTT specific example
// #define REQUEST_URL "broker.emqx.io"
// #define REQUEST_PATH "esp32/test"
// #define REQUEST_CLIENT_ID "esp-client-"
// #define REQUEST_USERNAME "emqx"
// #define REQUEST_PASSWORD "123"
// #endif // REQUEST_MODE
// // end request.h configs
//
// // Configs must come before the header inclusion
// #include "network.h"
// #include "request.h"
//
// NETWORK_INIT(network);
// REQUEST_INIT(network, request);
//
// void setup() {
//   Serial.begin(9600);
//   NETWORK_SETUP();
//   REQUEST_SETUP(request);
// }
//
// void loop() {
//   NETWORK_LOOP(); // ensures connection
//   REQUEST_LOOP(request); // ensures client's health
//   String data = "[data]";
//
//   DBG("\n\n");
//   Serial.println("Request results: ");
//   const bool is_ok = REQUEST_SEND(request, data);
//
//   Serial.print("Last connection: ");
//   Serial.println(is_ok ? "successful" : "failed");
//
//   delay(3000);
// }
// ```

#ifndef REQUEST_H_
#define REQUEST_H_

#include "network.h"

// Defaults
// Default mode (since HTTP is simpler and used widely)
#ifndef REQUEST_MODE
#define REQUEST_MODE 0
#endif // REQUEST_MODE

// Default port
#ifndef REQUEST_PORT
#if REQUEST_MODE == 0
#define REQUEST_PORT 80
#elif REQUEST_MODE == 1
#define REQUEST_PORT 1883
#endif // default request mode determination
#endif // REQUEST_PORT

// Default client id to NETWORK_MAC.c_str() if defined
#ifndef REQUEST_CLIENT_ID
#define MQTT_CLIENT_ID NETWORK_MAC.c_str()
#endif // REQUEST_CLIENT_ID

// Default method
#ifndef REQUEST_METHOD
#define REQUEST_METHOD "GET"
#endif // REQUEST_METHOD

// Default headers (see request_http)
#ifndef REQUEST_HEADERS
#define REQUEST_HEADERS ""
#endif // REQUEST_HEADERS

// Default request reply timeout
#ifndef REQUEST_REPLY_WAIT
#define REQUEST_REPLY_WAIT 100
#endif // REQUEST_REPLY_WAIT

// Dependecies
#ifndef DBG
#define DBG(...)
#endif // DBG

// Program
#if REQUEST_MODE == 0  // HTTP
#define _HEADER_LEN 49 // The header line length of the response
int _wait = 0;
/* Make a request and return response header.
 *
 * Includes Host header in all requests and Content-Length to POST methods.
 *
 * @param `method` must be in all caps.
 * @param NETWORK_CLIENT can either be EthernetClient or WiFiClient.
 * @returns 0 if request fails otherwise the http code.
 */
int http_request(String data, NETWORK_CLIENT &client, String method,
                 String base_url, String path, int port,
                 String additional_headers) {
  const bool not_get = !method.equals("GET");

  // Connect and make the request
  if (!NETWORK_CONNECT(client, base_url.c_str(), port))
    return 0;

  // Format request
  String request = "";
  request.concat(method);
  request.concat(" " + path);
  request.concat(not_get ? "" : ("?" + data));
  request.concat(" HTTP/1.1\n");
  request.concat("Host: " + base_url + "\n");
  if (not_get) {
    request.concat("Content-Length: ");
    request.concat(data.length());
    request.concat("\n");
  } // header end
  if (additional_headers != "" && additional_headers != NULL)
    request.concat(additional_headers);
  // data
  if (not_get)
    request.concat("\n" + data);

  DBG("Outgoing request:\n");
  client.println(request);
  DBG(request);
  DBG("\n");
  DBG("Outgoing request finished\n");

  DBG("HTTP response:\n");
  // Wait for the answer to come back just to be sure
  // Prevents some "empty response" instances
  while (client.available() == 0) {
    delay(1);
    if (_wait++ > REQUEST_REPLY_WAIT) {
      DBG("Wait for network timed out\n");
      _wait = 0;
      break;
    }
  }

  // Save the response header
  char header_str[_HEADER_LEN + 1];
  byte header_str_i = 0;
  while (NETWORK_CONNECTED(client)) {
    if (client.available()) {
      const char c = (char)client.read();
      if (header_str_i < _HEADER_LEN)
        header_str[header_str_i++] = c;
      DBG(c);
    } else
      // To prevent longer than necessary keep-alive's
      // BUG Connection: close are not correctly printed (stops on the first
      // letter)
      NETWORK_STOP(client);
  }
  header_str[header_str_i] = '\0';
  NETWORK_STOP(client);
  DBG("HTTP response finished\n");

  // Parse the header_str to extract the header
  String header = String(header_str);
  DBG("HTTP Response header: ");
  DBG(header);
  DBG("\n");

  const byte first_space = header.indexOf(" ");
  if (first_space == -1 || first_space > _HEADER_LEN)
    return 0;
  // Try to return "{xxx} WORD" where xxx is the http code (e.g. 200)
  int possible_code = header.substring(0, first_space).toInt();
  // If failed returning "HTTP/y {xxx} WORD"
  if (possible_code == 0)
    possible_code =
        header.substring(first_space + 1, first_space + 1 + 3).toInt();
  DBG("HTTP Code: ");
  DBG(possible_code);
  DBG("\n");
  return possible_code;
}
#define REQUEST_INIT(net_client, variable_name) /* just to suppress errors */  \
  NETWORK_CLIENT *variable_name = &net_client;
#define REQUEST_SETUP(client)
#define REQUEST_LOOP(client)
#define REQUEST_SEND(client, data)                                             \
  (0 != http_request(data, *client, String(REQUEST_METHOD),                    \
                     String(REQUEST_URL), "/" + String(REQUEST_PATH),          \
                     REQUEST_PORT, String(REQUEST_HEADERS)))

#elif REQUEST_MODE == 1 // MQTT

#include "PubSubClient.h"
#define REQUEST_INIT(net_client, variable_name)                                \
  PubSubClient variable_name(net_client)
#define REQUEST_SETUP(client)                                                  \
  client.setServer(REQUEST_URL, REQUEST_PORT);                                 \
  while (!client.connected())                                                  \
    if (client.connect(REQUEST_CLIENT_ID, REQUEST_USERNAME, REQUEST_PASSWORD)) \
      Serial.println("MQTT broker connected");                                 \
    else {                                                                     \
      Serial.print("failed with state ");                                      \
      Serial.println(client.state());                                          \
      delay(1000);                                                             \
    }
#define REQUEST_LOOP(client)                                                   \
  REQUEST_SETUP(client);                                                       \
  client.loop()
#define REQUEST_SEND(client, data)                                             \
  client.publish(REQUEST_PATH, data.c_str());                                  \
  DBG("Sent " + data + " to " + REQUEST_PATH + " topic on " + REQUEST_URL +    \
      "\n")

#endif // REQUEST_MODE

#endif // REQUEST_H_
