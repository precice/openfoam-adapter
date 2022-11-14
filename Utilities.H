#ifndef UTILITIES_H
#define UTILITIES_H

// Filter-out debug messages when not required
#ifdef ADAPTER_DEBUG_MODE
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

// Filter-out timing when not required
#ifdef ADAPTER_ENABLE_TIMINGS
#define TIMING_MODE(x) x
#else
#define TIMING_MODE(x)
#endif

// Shortcut for constructing clocks
#ifdef ADAPTER_ENABLE_TIMINGS
#define SETUP_TIMER() \
    clockValue clock; \
    clock.update()
#define REUSE_TIMER() clock.update()
#else
#define SETUP_TIMER()
#define REUSE_TIMER()
#endif


// Shortcut for adding time
#ifdef ADAPTER_ENABLE_TIMINGS
#define ACCUMULATE_TIMER(x) x += clock.elapsed()
#else
#define ACCUMULATE_TIMER(x)
#endif

// String added in the beginning of every printed message
#define INFO_STR_ADAPTER "---[preciceAdapter] "

#include "IOstreams.H"

void adapterInfo(const std::string message, const std::string level = "debug");

#endif
