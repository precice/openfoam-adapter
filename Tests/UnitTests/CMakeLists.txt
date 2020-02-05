########################################################################
#
# Project-wide settings

# Name of the project.
#
# CMake files in this project can refer to the root source directory
# as ${gtest_SOURCE_DIR} and to the root binary directory as
# ${gtest_BINARY_DIR}.
# Language "C" is required for find_package(Threads).

# Project version:
if (CMAKE_VERSION VERSION_LESS 3.0)
  project(gtest CXX C)
  set(PROJECT_VERSION ${GOOGLETEST_VERSION})
else()
  cmake_policy(SET CMP0048 NEW)
  project(gtest VERSION ${GOOGLETEST_VERSION} LANGUAGES CXX C)
endif()
cmake_minimum_required(VERSION 2.6.4)

set( CMAKE_CXX_STANDARD 11 )
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --no-as-needed -Xlinker --add-needed")

add_definitions(-Dlinux64 -DWM_ARCH_OPTION=64 -DUNIT_TESTING=1 -DWM_DP -DWM_LABEL_SIZE=32 -DNoRepository -m64 -fPIC -g)
link_directories($ENV{FOAM_LIBBIN})

find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
include_directories("../../")
include_directories($ENV{FOAM_SRC}/finiteVolume/lnInclude
        $ENV{FOAM_SRC}/meshTools/lnInclude
        $ENV{FOAM_SRC}/transportModels/
        $ENV{FOAM_SRC}/transportModels/incompressible/lnInclude
        $ENV{FOAM_SRC}/transportModels/compressible/lnInclude
        $ENV{FOAM_SRC}/transportModels/twoPhaseMixture/lnInclude
        $ENV{FOAM_SRC}/transportModels/interfaceProperties/lnInclude
        $ENV{FOAM_SRC}/transportModels/immiscibleIncompressibleTwoPhaseMixture/lnInclude
        $ENV{FOAM_SRC}/thermophysicalModels/basic/lnInclude
        $ENV{FOAM_SRC}/TurbulenceModels/turbulenceModels/lnInclude
        $ENV{FOAM_SRC}/TurbulenceModels/compressible/lnInclude
        $ENV{FOAM_SRC}/TurbulenceModels/incompressible/lnInclude
        $ENV{FOAM_SRC}/triSurface/lnInclude
        $ENV{FOAM_SRC}/OpenFOAM/lnInclude
        $ENV{FOAM_SRC}/OSspecific/POSIX
        $ENV{FOAM_SRC}/OSspecific/POSIX/cpuTime
        $ENV{FOAM_SRC}/OSspecific/POSIX/lnInclude)

include_directories(./MockFOAM ./MockAdapter ./MockpreCICE)

set(MOCK_ADAPTER_FILES)
set(MOCK_FOAM_FILES MockFOAM/fvMeshMock.cpp)
set(MOCK_PRECICE_FILES MockpreCICE/SolverInterfaceMock.cpp)
set(ADAPTER_FILES ../../Utilities.C ../../Adapter.C)
add_executable(TestFolder/Test1 ${MOCK_FOAM_FILES} ${MOCK_PRECICE_FILES} ${MOCK_ADAPTER_FILES} ${ADAPTER_FILES} AdapterTest/AdapterTest.cc)

set(EXTRA_LIBS dl m)
set(LIBS
        finiteVolume
        meshTools
        OpenFOAM
        compressibleTurbulenceModels
        incompressibleTurbulenceModels
        immiscibleIncompressibleTwoPhaseMixture
        gtest
        gmock
        ${EXTRA_LIBS})

target_link_libraries(TestFolder/Test1 ${GTEST_BOTH_LIBRARIES} precice yaml-cpp ${LIBS})