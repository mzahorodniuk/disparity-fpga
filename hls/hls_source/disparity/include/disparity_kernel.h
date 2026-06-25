#include "xf_stereolbm_config.h"
#include "imgproc/xf_stereolbm.hpp"
#include "core/xf_convert_bitdepth.hpp"
#include "imgproc/xf_convertscaleabs.hpp"
#include "stereo_cam_config.h"


void disparity_kernel(ap_uint<PTR_IN_WIDTH>* img_in_l,
                     ap_uint<PTR_IN_WIDTH>* img_in_r,
                     unsigned int* bm_state_in,
                     ap_uint<PTR_OUT_WIDTH>* img_out,
                     int height,
                     int width);