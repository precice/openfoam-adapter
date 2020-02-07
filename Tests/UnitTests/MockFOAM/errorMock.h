//
// Created by keefe on 1/2/20.
//

#ifndef ERROR_H
#define ERROR_H
#include <string>

namespace Foam{
    class error
    :
        public std::exception
    {
    public:
        string message() const{
            return "Error message";
        }

    };
}


#endif //GTEST_ERRORMOCK_H
