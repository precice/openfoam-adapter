#ifndef UTILITIES_H
#define UTILITIES_H

// Output debug messages. Keep it disabled for production runs.
// #define ADAPTER_DEBUG_MODE

#ifdef ADAPTER_DEBUG_MODE
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

// String added in the beginning of every printed message
#define INFO_STR_ADAPTER "---[preciceAdapter] "

#include "IOstreams.H"

void adapterInfo(const std::string message, const std::string level = "debug");

#endif
