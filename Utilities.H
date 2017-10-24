#ifndef UTILITIES_H
#define UTILITIES_H

// Filter-out debug messages when not required
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
