//
// Created by keefe on 1/2/20.
//

#ifndef fileName_H
#define fileName_H


class fileName {
public:
    enum{
        UNDEFINED,
        FILE,
        DIRECTORY,
        LINK
    };

    explicit fileName(const Foam::string foam_string){};
};


#endif //fileName_H
