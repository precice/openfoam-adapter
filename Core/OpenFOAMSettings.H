
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

// dictonary lookup functions
#ifdef __OF_DICT_LOOKUPFUNCTION__
#error c++ macro __OF_DICT_LOOKUPFUNCTION__ is already defined.
#endif

#if   (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotCOM) && (OpenFOAM_VERSION_MAJOR >= 1806)
#define __OF_DICT_LOOKUPFUNCTION__(dict, type, pname) dict.get<type>(pname);
#elif (OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR >= 8)
#define __OF_DICT_LOOKUPFUNCTION__(dict, type, pname) dict.lookup<type>(pname);
#elif ((OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotORG) && (OpenFOAM_VERSION_MAJOR > 4)) || \
      ((OpenFOAM_VENDOR == OpenFOAM_VENDOR_dotCOM) && (OpenFOAM_VERSION_MAJOR >= 1706))
#define __OF_DICT_LOOKUPFUNCTION__(dict, type, pname) dict.lookupType<type>(pname);
#else
#define __OF_DICT_LOOKUPFUNCTION__(dict, type, pname) static_cast<type>(dict.lookup(pname));
#endif

#endif
