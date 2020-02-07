//
// Created by keefe on 6/2/20.
//

#include "UtilitiesMock.h"
#include <functional>

static std::function<void(const std::string, const std::string)> _adapterInfo;

adapterInfoMock::adapterInfoMock(){
    assert(!_adapterInfo);
    _adapterInfo = [this](const std::string message, const std::string level){return adapterInfo(message, level);};
}

adapterInfoMock::~adapterInfoMock(){
    _adapterInfo = {};
}

void adapterInfo(const std::string message, const std::string level){
    return _adapterInfo(message, level);
};
