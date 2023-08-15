/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | www.openfoam.com
     \\/     M anipulation  |
-------------------------------------------------------------------------------
    Copyright (C) 2020 OpenCFD Ltd.
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

// * * * * * * * * * * * * * Static Member Functions * * * * * * * * * * * * //

inline Foam::clockValue Foam::clockValue::now()
{
    return clockValue(true);
}


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

inline Foam::clockValue::clockValue()
:
    value_(value_type::zero())
{}


inline Foam::clockValue::clockValue(bool)
:
    value_(std::chrono::high_resolution_clock::now().time_since_epoch())
{}


inline Foam::clockValue::clockValue(const value_type& value)
:
    value_(value)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

inline void Foam::clockValue::clear()
{
    value_ = value_type::zero();
}


inline void Foam::clockValue::update()
{
    value_ = std::chrono::high_resolution_clock::now().time_since_epoch();
}


inline long Foam::clockValue::seconds() const
{
    return std::chrono::duration_cast<std::chrono::seconds>(value_).count();
}


inline Foam::clockValue Foam::clockValue::elapsed() const
{
    return (clockValue::now() -= *this);
}


inline double Foam::clockValue::elapsedTime() const
{
    return static_cast<double>(this->elapsed());
}


// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

inline Foam::clockValue::operator double() const
{
    return
    (
        (double(value_.count()) * value_type::period::num)
      / value_type::period::den
    );
}


inline Foam::clockValue& Foam::clockValue::operator-=(const clockValue& rhs)
{
    value_ -= rhs.value_;
    return *this;
}


inline Foam::clockValue& Foam::clockValue::operator+=(const clockValue& rhs)
{
    value_ += rhs.value_;
    return *this;
}


// ************************************************************************* //
