// Dynamic Logging Module
//
// Define DEBUG_MODE to be 1 before importing this header to use the DBG.
//
// Regardless of DEBUG config will define the following for the user (note that
// for macro functions, the signitures are symmetrical between both variants):
// - DEBUG(msg): Prints the log message in the serial if DEBUG is true.

#ifndef DEBUG_H_
#define DEBUG_H_

// Default mode
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif // DEBUG_MODE

#if DEBUG_MODE == 1
#define DBG(msg) Serial.print(msg)
#else
#define DBG(msg)
#endif // DEBUG_MODE

#endif // DEBUG_H_
