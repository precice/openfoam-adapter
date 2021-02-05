
#ifndef OpenFOAMSETTINGS_H
#define OpenFOAMSETTINGS_H

// OpenFOAM_VENDOR values are
#define OpenFOAM_VENDOR_Unknown 0
#define OpenFOAM_VENDOR_dotCOM  1
#define OpenFOAM_VENDOR_dotORG  2

// checks for requirements
#ifndef OpenFOAM_VENDOR
#error  OpenFOAM_VENDOR are not set.
#endif

#ifndef OpenFOAM_VERSION
#error  OpenFOAM_VERSION are not set.
#endif

#ifndef OpenFOAM_VERSION_MAJOR
#error  OpenFOAM_VERSION_MAJOR are not set.
#endif

// not used atm --> not needed actually (can be changed every time)
//#ifndef OpenFOAM_VERSION_MINOR
//#error  OpenFOAM_VERSION_MINOR are not set.
//#endif
//
//#ifndef OpenFOAM_VERSION_REVISION
//#error  OpenFOAM_VERSION_REVISION are not set.
//#endif

#endif
