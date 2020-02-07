//
// Created by keefe on 5/2/20.
//

#ifndef GTEST_CONSTANTSMOCK_H
#define GTEST_CONSTANTSMOCK_H
#include <string>

namespace Foam{
    typedef std::string word;
    typedef std::string string;
    typedef double scalar;
    typedef int label;

    static const scalar GREAT = 1.0e+6;

    template<class PrimitiveType>
    class pTraits{
        explicit pTraits(const PrimitiveType& p){};
    };
}

#endif //GTEST_CONSTANTSMOCK_H
