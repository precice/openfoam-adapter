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

\*---------------------------------------------------------------------------*/

#include "clockValue.H"
#include <sstream>
#include <iomanip>

// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

std::string Foam::clockValue::str() const
{
    std::ostringstream os;

    // seconds
    const unsigned long ss =
         std::chrono::duration_cast<std::chrono::seconds>(value_).count();

    // days
    const auto dd = (ss / 86400);

    // hours
    const int hh = ((ss / 3600) % 24);

    if (dd) os << dd << '-';

    if (dd || hh)
    {
        os  << std::setw(2) << std::setfill('0')
            << hh << ':';
    }

    // minutes
    os  << std::setw(2) << std::setfill('0')
        << ((ss / 60) % 60) << ':';

    // seconds
    os  << std::setw(2) << std::setfill('0')
        << (ss % 60);

    // milliseconds. As none or 3 decimal places
    const long ms =
    (
        std::chrono::duration_cast<std::chrono::milliseconds>(value_).count()
      - (ss * 1000)
    );

    if (ms > 0)
    {
        os  << '.' << std::setw(3) << std::setfill('0') << ms;
    }

    return os.str();
}


// ************************************************************************* //
