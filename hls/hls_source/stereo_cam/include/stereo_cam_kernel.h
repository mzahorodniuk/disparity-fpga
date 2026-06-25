#include "xf_stereocam_config.h"

#include "stereo_cam_config.h"

#include "hls_stream.h"


#include "common/xf_common.hpp"
#include "common/xf_utility.hpp"
#include "common/xf_structs.hpp"
#include "imgproc/xf_stereolbm.hpp"
#include "imgproc/xf_median_blur.hpp"
#include "imgproc/xf_stereo_pipeline.hpp"

#include "imgproc/xf_remap.hpp"
#include "core/xf_convert_bitdepth.hpp"
#include "imgproc/xf_convertscaleabs.hpp"



void stereo_cam_kernel(
    ap_uint<PTR_IN_WIDTH>* img_L,
    ap_uint<PTR_IN_WIDTH>* img_R,
    ap_uint<PTR_OUT_WIDTH>* img_disp,
    float* cameraMA_l,
    float* cameraMA_r,
    float* distC_l,
    float* distC_r,
    float* irA_l,
    float* irA_r,
    unsigned int* bm_state_arr,
    int height,
    int width
);