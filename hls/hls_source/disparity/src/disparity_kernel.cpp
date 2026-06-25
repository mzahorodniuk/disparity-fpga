#include "disparity_kernel.h"


void disparity_kernel(ap_uint<PTR_IN_WIDTH>* img_in_l,
                     ap_uint<PTR_IN_WIDTH>* img_in_r,
                     unsigned int* bm_state_in,
                     ap_uint<PTR_OUT_WIDTH>* img_out,
                     int height,
                     int width) {
// clang-format off
	#pragma HLS INTERFACE m_axi      port=img_in_l      offset=slave  bundle=gmem0
	#pragma HLS INTERFACE m_axi      port=img_in_r      offset=slave  bundle=gmem1
	#pragma HLS INTERFACE m_axi      port=bm_state_in   offset=slave  bundle=gmem2
	#pragma HLS INTERFACE m_axi      port=img_out       offset=slave  bundle=gmem3
	#pragma HLS INTERFACE s_axilite  port=height
	#pragma HLS INTERFACE s_axilite  port=width
	#pragma HLS INTERFACE s_axilite  port=return
    // clang-format on

    xf::cv::Mat<IN_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC> imgInputL(height, width);
    xf::cv::Mat<IN_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC> imgInputR(height, width);
    xf::cv::Mat<XF_16UC1, IMG_HEIGHT, IMG_WIDTH, NPC> imgOutputStereo16(height, width);
    xf::cv::Mat<OUT_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC> imgOutputStereo8(height, width);
    xf::cv::Mat<OUT_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC> imgOutputMedian(height, width);

    xf::cv::xFSBMState<SAD_WINDOW_SIZE, NO_OF_DISPARITIES, PARALLEL_UNITS> bmState;

    // Initialize SBM State:
    bmState.preFilterCap = bm_state_in[0];
    bmState.uniquenessRatio = bm_state_in[1];
    bmState.textureThreshold = bm_state_in[2];
    bmState.minDisparity = bm_state_in[3];

// clang-format off
	#pragma HLS STREAM variable=imgInputL.data depth=2
	#pragma HLS STREAM variable=imgInputR.data depth=2
	#pragma HLS STREAM variable=imgOutputMedian.data depth=2
// clang-format on

// clang-format off
	#pragma HLS DATAFLOW
    // clang-format on

    // Retrieve xf::Mat objects from img_in data:
    xf::cv::Array2xfMat<PTR_IN_WIDTH, IN_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC>(img_in_l, imgInputL);
    xf::cv::Array2xfMat<PTR_IN_WIDTH, IN_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC>(img_in_r, imgInputR);

    // Run xfOpenCV kernel:
    xf::cv::StereoBM<SAD_WINDOW_SIZE, NO_OF_DISPARITIES, PARALLEL_UNITS, IN_TYPE, XF_16UC1, IMG_HEIGHT, IMG_WIDTH, NPC,
                     XF_USE_URAM>(imgInputL, imgInputR, imgOutputStereo16, bmState);

    xf::cv::convertTo<XF_16UC1,OUT_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC>(imgOutputStereo16,imgOutputStereo8,XF_CONVERT_16U_TO_8U,1);

    xf::cv::medianBlur<WINDOW_SIZE, XF_BORDER_REPLICATE, OUT_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC1>(imgOutputStereo8, imgOutputMedian);

    // Convert _dst xf::Mat object to output array:
    xf::cv::xfMat2Array<PTR_OUT_WIDTH, OUT_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC>(imgOutputMedian, img_out);

    return;
} // End of kernel

