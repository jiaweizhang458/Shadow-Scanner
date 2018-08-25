/***********************************************************************************
 Header file:
 LVUVCPublic.h

 Description:

 The Logitech UVC Driver Public Property Sets are a collection of
 property sets designed for use by third party application
 developers. This include file contains the necessary interface
 definitions to use these property sets.

 Please refer to the document "Logitech UVC Driver Public Property
 Sets: Specification" available from http://www.quickcamteam.net/ for
 details.

 Version: 1.0

 Copyright (c) 2009 Logitech,  All Rights Reserved
***********************************************************************************/

#ifndef LVUVCPUBLIC_H
#define LVUVCPUBLIC_H

//#include <KS.h>

#pragma warning(disable:4201) // nameless struct/union
#pragma warning(disable:4214) // bit field types other than int

#if defined(__TCS__)
#define _KS_NO_ANONYMOUS_STRUCTURES_ 1
#endif

#if !defined(_NTRTL_)
#ifndef DEFINE_GUIDEX
#define DEFINE_GUIDEX(name) EXTERN_C const CDECL GUID name
#endif // !defined(DEFINE_GUIDEX)

#ifndef STATICGUIDOF
#define STATICGUIDOF(guid) STATIC_##guid
#endif // !defined(STATICGUIDOF)
#endif // !defined(_NTRTL_)

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(ar) (sizeof(ar)/sizeof((ar)[0]))
#endif // !defined(SIZEOF_ARRAY)

#if defined(__cplusplus) && _MSC_VER >= 1100
#define DEFINE_GUIDSTRUCT(g, n) struct __declspec(uuid(g)) n
#define DEFINE_GUIDNAMED(n) __uuidof(struct n)
#else // !defined(__cplusplus)
#define DEFINE_GUIDSTRUCT(g, n) DEFINE_GUIDEX(n)
#define DEFINE_GUIDNAMED(n) n
#endif // !defined(__cplusplus)

/*******************************************************************************
 * GUIDS
 ******************************************************************************/

// {CAAE4966-272C-44a9-B792-71953F89DB2B}
#define STATIC_PROPSETID_LOGITECH_PUBLIC1                               \
  0xCAAE4966, 0x272C, 0x44A9, 0xB7, 0x92, 0x71, 0x95, 0x3F, 0x89, 0xDB, 0x2B
DEFINE_GUIDSTRUCT("CAAE4966-272C-44a9-B792-71953F89DB2B", PROPSETID_LOGITECH_PUBLIC1);
#define PROPSETID_LOGITECH_PUBLIC1 DEFINE_GUIDNAMED(PROPSETID_LOGITECH_PUBLIC1)

/*******************************************************************************
 * ENUMERATIONS AND CONSTANTS
 ******************************************************************************/

typedef enum _KSPROPERTY_LP1_PROPERTY {
  KSPROPERTY_LP1_VERSION,
  KSPROPERTY_LP1_DIGITAL_PAN,
  KSPROPERTY_LP1_DIGITAL_TILT,
  KSPROPERTY_LP1_DIGITAL_ZOOM,
  KSPROPERTY_LP1_DIGITAL_PANTILTZOOM,
  KSPROPERTY_LP1_EXPOSURE_TIME,
  KSPROPERTY_LP1_FACE_TRACKING,
  KSPROPERTY_LP1_LED,
  KSPROPERTY_LP1_FINDFACE,

  KSPROPERTY_LP1_LAST = KSPROPERTY_LP1_FINDFACE

} KSPROPERTY_LP1_PROPERTY;

typedef enum _LVUVC_LP1_FACE_TRACKING_MODE {
  LVUVC_LP1_FACE_TRACKING_MODE_OFF,
  LVUVC_LP1_FACE_TRACKING_MODE_SINGLE,
  LVUVC_LP1_FACE_TRACKING_MODE_MULTIPLE,
} LVUVC_LP1_FACE_TRACKING_MODE;

typedef enum _LVUVC_LP1_LED_MODE {
  LVUVC_LP1_LED_MODE_OFF,
  LVUVC_LP1_LED_MODE_ON,
  LVUVC_LP1_LED_MODE_BLINKING,
  LVUVC_LP1_LED_MODE_AUTO,
} LVUVC_LP1_LED_MODE;

typedef enum _LVUVC_LP1_FINDFACE_MODE {
  LVUVC_LP1_FINDFACE_MODE_NO_CHANGE,
  LVUVC_LP1_FINDFACE_MODE_OFF,
  LVUVC_LP1_FINDFACE_MODE_ON,
} LVUVC_LP1_FINDFACE_MODE;

typedef enum _LVUVC_LP1_FINDFACE_RESET {
  LVUVC_LP1_FINDFACE_RESET_NONE,
  LVUVC_LP1_FINDFACE_RESET_DEFAULT,
  LVUVC_LP1_FINDFACE_RESET_FACE,
} LVUVC_LP1_FINDFACE_RESET;

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct _KSPROPERTY_LP1_HEADER {
  ULONG ulFlags;
  ULONG ulReserved1;
  ULONG ulReserved2;
} KSPROPERTY_LP1_HEADER, *PKSPROPERTY_LP1_HEADER;

typedef struct _KSPROPERTY_LP1_VERSION_S {
  KSPROPERTY_LP1_HEADER	Header;
  USHORT usMajor;
  USHORT usMinor;
} KSPROPERTY_LP1_VERSION_S, *PKSPROPERTY_LP1_VERSION_S;

typedef struct _KSPROPERTY_LP1_DIGITAL_PAN_S {
  KSPROPERTY_LP1_HEADER	Header;
  LONG lPan;
} KSPROPERTY_LP1_DIGITAL_PAN_S, *PKSPROPERTY_LP1_DIGITAL_PAN_S;

typedef struct _KSPROPERTY_LP1_DIGITAL_TILT_S {
  KSPROPERTY_LP1_HEADER	Header;
  LONG lTilt;
} KSPROPERTY_LP1_DIGITAL_TILT_S, *PKSPROPERTY_LP1_DIGITAL_TILT_S;

typedef struct _KSPROPERTY_LP1_DIGITAL_ZOOM_S {
  KSPROPERTY_LP1_HEADER	Header;
  ULONG ulZoom;
} KSPROPERTY_LP1_DIGITAL_ZOOM_S, *PKSPROPERTY_LP1_DIGITAL_ZOOM_S;

typedef struct _KSPROPERTY_LP1_DIGITAL_PANTILTZOOM_S {
  KSPROPERTY_LP1_HEADER	Header;
  LONG  lPan;
  LONG  lTilt;
  ULONG ulZoom;
} KSPROPERTY_LP1_DIGITAL_PANTILTZOOM_S, *PKSPROPERTY_LP1_DIGITAL_PANTILTZOOM_S;

typedef struct _KSPROPERTY_LP1_EXPOSURE_TIME_S {
  KSPROPERTY_LP1_HEADER	Header;
  ULONG ulExposureTime;
} KSPROPERTY_LP1_EXPOSURE_TIME_S, *PKSPROPERTY_LP1_EXPOSURE_TIME_S;

typedef struct _KSPROPERTY_LP1_FACE_TRACKING_S {
  KSPROPERTY_LP1_HEADER	Header;
  ULONG ulMode;			// See LVUVC_LP1_FACE_TRACKING_MODE
} KSPROPERTY_LP1_FACE_TRACKING_S, *PKSPROPERTY_LP1_FACE_TRACKING_S;

typedef struct _KSPROPERTY_LP1_LED_S {
  KSPROPERTY_LP1_HEADER	Header;
  ULONG ulMode;			// See LVUVC_LP1_LED_MODE
  ULONG ulFrequency;
} KSPROPERTY_LP1_LED_S, *PKSPROPERTY_LP1_LED_S;

typedef struct _KSPROPERTY_LP1_FINDFACE_S {
  KSPROPERTY_LP1_HEADER	Header;
  ULONG ulMode;			// See LVUVC_LP1_FINDFACE_MODE
  ULONG ulReset;		// See LVUVC_LP1_FINDFACE_RESET
} KSPROPERTY_LP1_FINDFACE_S, *PKSPROPERTY_LP1_FINDFACE_S;

#endif // #ifndef LVUVCPUBLIC_H
