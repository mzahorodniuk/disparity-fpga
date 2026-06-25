#ifndef _XF_STEREOBM_CONFIG_H_
#define _XF_STEREOBM_CONFIG_H_

#include "xf_config_params.h"

// Set the input and output pixel depth:
#define IN_TYPE XF_8UC1
//#define IN_TYPE XF_16UC1

#define PTR_IN_WIDTH 8
//#define PTR_IN_WIDTH 128

#define OUT_TYPE XF_8UC1
#define PTR_OUT_WIDTH 8
//#define PTR_OUT_WIDTH 64

#define XF_CV_DEPTH_MAP_XL XF_16SC1
#define XF_CV_DEPTH_MAP_YL XF_16SC1
#define XF_CV_DEPTH_MAP_XR XF_16SC1
#define XF_CV_DEPTH_MAP_YR XF_16SC1

#define XF_CV_DEPTH_LEFT_REMAPPED XF_16SC1
#define XF_CV_DEPTH_RIGHT_REMAPPED XF_16SC1

#define XF_REMAP_BUFSIZE 4


// Set the optimization type:
#define NPC XF_NPPC1
#define NPC1 XF_NPPC1

/*
// Set the optimization type:
#if NO == 1
#define NPC1 XF_NPPC1
#define PTR_WIDTH 32
#else

#if GRAY
#define NPC1 XF_NPPC8
#else
#define NPC1 XF_NPPC4
#endif

#define PTR_WIDTH 128
#endif


// Set the pixel depth:
#if GRAY
#define TYPE XF_8UC1
#else
#define TYPE XF_8UC3
#endif
*/

#endif // _XF_STEREOBM_CONFIG_H_
