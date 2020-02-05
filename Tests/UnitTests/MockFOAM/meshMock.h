//
// Created by keefe on 1/2/20.
//

#ifndef GTEST_MESHMOCK_H
#define GTEST_MESHMOCK_H

namespace Foam {
    class TimeMock{
    public:
        int oldTime(){
            return 0;
        }

        bool operator==(const TimeMock &c1) {
            return true;
        }
    };

    class pointField {
    public:

        pointField()=default;
        ~pointField()=default;
        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }
    };

    class surfaceScalarField {
    public:
        surfaceScalarField()=default;
        ~surfaceScalarField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }


        bool operator==(const surfaceScalarField &c1) {
            return true;
        }
    };

    class surfaceVectorField {
    public:
        surfaceVectorField()=default;
        ~surfaceVectorField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        bool operator==(const surfaceVectorField &c1) {
            return true;
        }
    };

    class volVectorField {
    public:
        volVectorField()=default;
        ~volVectorField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        void correctBoundaryConditions() {};

        bool operator==(const volVectorField &c1) {
            return true;
        }
    };

    class volScalarField {
    public:
        volScalarField()=default;
        ~volScalarField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        std::string name(){
            return "";
        }

        void correctBoundaryConditions() {};

        bool operator==(const volScalarField &c1) {
            return true;
        }

    public:
        class Internal {
        public:
            Internal()=default;
            ~Internal()=default;

        };

    };

    class pointScalarField {
    public:
        pointScalarField()=default;
        ~pointScalarField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        void correctBoundaryConditions() {};

        bool operator==(const pointScalarField &c1) {
            return true;
        }

    };

    class pointVectorField {
    public:
        pointVectorField()=default;
        ~pointVectorField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        void correctBoundaryConditions() {};

        bool operator==(const pointVectorField &c1) {
            return true;
        }
    };

    class surfaceTensorField {
    public:
        surfaceTensorField()=default;
        ~surfaceTensorField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        bool operator==(const surfaceTensorField &c1) {
            return true;
        }
    };

    class volTensorField {
    public:
        volTensorField()=default;
        ~volTensorField()=default;

        int nOldTimes() {

            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        bool operator==(const volTensorField &c1) {
            return true;
        }
    };

    class pointTensorField {
    public:
        pointTensorField()=default;
        ~pointTensorField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        bool operator==(const pointTensorField &c1) {
            return true;
        }

    };

    class volSymmTensorField {
    public:
        volSymmTensorField()=default;
        ~volSymmTensorField()=default;

        int nOldTimes() {
            return 0;
        }

        TimeMock time;
        TimeMock& oldTime(){
            return time;
        }

        bool operator==(const volSymmTensorField &c1) {
            return true;
        }

    };

}

#endif //GTEST_MESHMOCK_H
