//
// Created by keefe on 1/2/20.
//

#ifndef PSTREAM_H
#define PSTREAM_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

class Pstream {
public:
    static int myProcNo(){
        return 0;
    };

    static int nProcs(){
        return 1;
    };
};


#endif //GTEST_PSTREAM_H
