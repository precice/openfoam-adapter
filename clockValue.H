/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2018-2020 OpenCFD Ltd.
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM.  If not, see <http://www.gnu.org/licenses/>.

Class
    Foam::clockValue

Description
    Access to high-resolution clock value with some basic operations.
    Used to calculate time durations, elapsed times etc.

SourceFiles
    clockValueI.H
    clockValue.C

\*---------------------------------------------------------------------------*/

#ifndef clockValue_H
#define clockValue_H

#include <chrono>
#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

/*---------------------------------------------------------------------------*\
                         Class clockValue Declaration
\*---------------------------------------------------------------------------*/

class clockValue
{
public:

    // Public Types

        //- Time structure used
        typedef std::chrono::high_resolution_clock::duration value_type;


private:

    // Private Data

        //- The time start point or the time duration.
        value_type value_;


public:

    // Constructors

        //- Construct zero initialized
        inline clockValue();

        //- Construct with current time.
        //  The bool is for tagged dispatch only (its value is ignored).
        inline explicit clockValue(bool);

        //- Copy construct from duration with the same clock base
        inline explicit clockValue(const value_type& value);


    // Factory Methods

        //- The current clock value from the system
        inline static clockValue now();


    // Member Functions

        //- The time duration
        inline const value_type& value() const
        {
            return value_;
        }

        //- Reset to zero
        inline void clear();

        //- Update to the current now() time from the system
        inline void update();

        //- The value in seconds (rounded)
        inline long seconds() const;

        //- The time duration elapsed until now() since the start point
        inline clockValue elapsed() const;

        //- The time elapsed [seconds] until now() since the start point
        inline double elapsedTime() const;

        //- Format as day-hh:mm:ss string
        std::string str() const;


    // Operators

        //- Conversion operator to seconds in floating point
        inline operator double() const;

        //- Subtract clock value
        inline clockValue& operator-=(const clockValue& rhs);

        //- Add clock value
        inline clockValue& operator+=(const clockValue& rhs);
};


// Global Operators

//- Subtraction of clock values
inline clockValue operator-(const clockValue& a, const clockValue& b)
{
    return clockValue(a.value() - b.value());
}

//- Addition of clock values
inline clockValue operator+(const clockValue& a, const clockValue& b)
{
    return clockValue(a.value() + b.value());
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#include "clockValueI.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

#endif

// ************************************************************************* //
