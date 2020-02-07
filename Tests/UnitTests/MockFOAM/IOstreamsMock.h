//
// Created by keefe on 1/2/20.
// Names IOstreams.H to replace includes in OpenFOAM adapter '.C' files.
//

#ifndef IOstreams_H
#define IOstreams_H

#include "errorMock.h"

#define forAll(list, i) \
    for (Foam::label i=0; i<(list).size(); i++) // NOLINT: Define for loop per OpenFOAM C++ definition

#endif //GTEST_IOSTREAMSMOCK_H
