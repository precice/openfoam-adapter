//
// Created by keefe on 6/2/20.
//

#ifndef GTEST_UTILITIES_H
#define GTEST_UTILITIES_H

#ifdef ADAPTER_DEBUG_MODE
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif

#include <string>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

void adapterInfo(const std::string message, const std::string level = "debug");

#endif //GTEST_UTILITIES_H

#ifndef UTILITIES_H
#define UTILITIES_H

#endif
