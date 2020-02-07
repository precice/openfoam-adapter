#ifndef Time_H
#define Time_H

#include "constantsMock.h"
#include <list>
#include "PstreamMock.h"
#include "IOstreamsMock.h"
#include "fileNameMock.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //


namespace Foam
{
    constexpr float TEST_TIME_STEP = 0.1234f;

    // Type declaration for Foam::list
    typedef std::vector<Foam::word> wordList;

/*---------------------------------------------------------------------------*\
                             Class Time Declaration
\*---------------------------------------------------------------------------*/

// Mocked unwatechedIOdictionary class. Has mock member function lookupOrDefault
class unwatchedIOdictionaryMock {
    public:
        unwatchedIOdictionaryMock()=default;
        unwatchedIOdictionaryMock(unwatchedIOdictionaryMock const &mock){};
        ~unwatchedIOdictionaryMock()=default;

        unwatchedIOdictionaryMock& operator=(const unwatchedIOdictionaryMock& dict){ // NOLINT: Created operator= stub for testing
            return *this;
        }
        MOCK_METHOD(bool, lookupOrDefault, (std::string, bool));
    };

// Mocked timePath class. Has mock member function type()
class timePathMock {
public:
    timePathMock()=default;
    timePathMock(const timePathMock& mock){};
    ~timePathMock()=default;
    MOCK_METHOD(int, type, (), (const));
};

// Mocked arbitrary class. Has mock member function toc()
// Returned by default when lookupClass mocked method is called.
class classMock{
public:
    classMock()=default;
    classMock(const classMock& mock) {};

    ~classMock()=default;
    MOCK_METHOD(wordList, tocMock, (), (const));
    wordList toc() const{
        EXPECT_CALL(*this, tocMock()).Times(1);
        return tocMock();
    };
    MOCK_METHOD(void, end, (), (const));
};

class Time
{

public:

    //- Stop-run control options
    enum stopAtControls
    {
        saEndTime,    //!< stop when Time reaches the prescribed endTime
        saNoWriteNow, //!< set endTime to stop immediately w/o writing
        saWriteNow,   //!< set endTime to stop immediately w/ writing
        saNextWrite   //!< stop the next time data are written
    };

    //- Supported time directory name formats
    enum fmtflags
    {
        general    = 0,
        fixed      = 1,
        scientific = 2
    };

    //- Construct given name of dictionary to read and argument list
    Time()=default;
    //- Destructor
    ~Time()=default;
    Time(const Time& time){};

    MOCK_METHOD(label, timeIndex, (), (const));

    MOCK_METHOD(bool, stopAt, (stopAtControls), (const));

    //- Read control dictionary, update controls and time
    MOCK_METHOD(bool, read, ());

    //- Read the objects that have been modified
    MOCK_METHOD(void, readModifiedObjects, ());

    //- Write time dictionary to the \<time\>/uniform directory
    MOCK_METHOD(bool, writeTimeDict, (), (const));

    //- Write the objects now (not at end of iteration) and continue
    //  the run
    MOCK_METHOD(bool, writeNow, ());

    //- Method to get controldict
    MOCK_METHOD(unwatchedIOdictionaryMock&, controlDict, (), (const));

    MOCK_METHOD(void, setEndTime, (scalar), (const));

    MOCK_METHOD(const timePathMock&, timePath, (), (const));

    MOCK_METHOD(Time&, deltaT, (), (const));

    MOCK_METHOD(void, setDeltaT, (double, bool), (const));

    MOCK_METHOD(void, setTime, (Foam::scalar, Foam::label), (const));

    MOCK_METHOD(bool, runTimeModifiable, (), (const));

    MOCK_METHOD(classMock&, functionObjects, ());

    MOCK_METHOD(bool, subCycling, (), (const));

    MOCK_METHOD(scalar, value, (), (const));

    MOCK_METHOD(bool, processorCase, (), (const));

    MOCK_METHOD(std::string, path, (), (const));
};


} // End namespace Foam

#endif

