//
// Created by keefe on 1/2/20.
//

#ifndef MESSAGESTREAM_H
#define MESSAGESTREAM_H



namespace Foam{
#define WarningInFunction Info;
    static const char nl = '\n';
    class Info{
        void operator<< (const std::string&){}
        void operator<< (const char* character){}
    };
}

#endif //GTEST_MESSAGESTREAMMOCK_H
