#include "stereo_cam_kernel.h"

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
) {

#pragma HLS INTERFACE m_axi     port=img_L          offset=slave bundle=gmem1
#pragma HLS INTERFACE m_axi     port=img_R          offset=slave bundle=gmem5
#pragma HLS INTERFACE m_axi     port=img_disp       offset=slave bundle=gmem6
#pragma HLS INTERFACE m_axi     port=cameraMA_l     offset=slave bundle=gmem2
#pragma HLS INTERFACE m_axi     port=cameraMA_r     offset=slave bundle=gmem2
#pragma HLS INTERFACE m_axi     port=distC_l        offset=slave bundle=gmem3
#pragma HLS INTERFACE m_axi     port=distC_r        offset=slave bundle=gmem3
#pragma HLS INTERFACE m_axi     port=irA_l          offset=slave bundle=gmem2
#pragma HLS INTERFACE m_axi     port=irA_r          offset=slave bundle=gmem2
#pragma HLS INTERFACE m_axi     port=bm_state_arr   offset=slave bundle=gmem4
#pragma HLS INTERFACE s_axilite port=height
#pragma HLS INTERFACE s_axilite port=width
#pragma HLS INTERFACE s_axilite port=return

        ap_fixed<32, 12> cameraMA_l_fix[CAMERA_MATRIX_SIZE], cameraMA_r_fix[CAMERA_MATRIX_SIZE],
                distC_l_fix[DIST_COEFF_SIZE], distC_r_fix[DIST_COEFF_SIZE], irA_l_fix[CAMERA_MATRIX_SIZE],
                irA_r_fix[CAMERA_MATRIX_SIZE];

        for (int i = 0; i < CAMERA_MATRIX_SIZE; i++) {

                #pragma HLS PIPELINE II=1
                // clang-format on
                cameraMA_l_fix[i] = (ap_fixed<32, 12>)cameraMA_l[i];
                cameraMA_r_fix[i] = (ap_fixed<32, 12>)cameraMA_r[i];
                irA_l_fix[i] = (ap_fixed<32, 12>)irA_l[i];
                irA_r_fix[i] = (ap_fixed<32, 12>)irA_r[i];
        }
        for (int i = 0; i < DIST_COEFF_SIZE; i++) {

                #pragma HLS PIPELINE II=1
                // clang-format on
                distC_l_fix[i] = (ap_fixed<32, 12>)distC_l[i];
                distC_r_fix[i] = (ap_fixed<32, 12>)distC_r[i];
        }

        xf::cv::xFSBMState<SAD_WINDOW_SIZE, NO_OF_DISPARITIES, PARALLEL_UNITS> bm_state;
        
        bm_state.preFilterSize  = bm_state_arr[0];
        bm_state.preFilterCap = bm_state_arr[1];
        bm_state.SADWindowSize = bm_state_arr[2];
        bm_state.minDisparity = bm_state_arr[3];
        bm_state.numberOfDisparities = bm_state_arr[4];
        bm_state.textureThreshold = bm_state_arr[5];
        bm_state.uniquenessRatio = bm_state_arr[6];
        bm_state.ndisp_unit = bm_state_arr[7];
        bm_state.sweepFactor = bm_state_arr[8];
        bm_state.remainder = bm_state_arr[9];


        xf::cv::Mat<XF_8UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAT_L> mat_L(height, width);

        xf::cv::Mat<XF_8UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAT_R> mat_R(height, width);

        xf::cv::Mat<XF_16UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAT_DISP> mat_disp(height, width);

        xf::cv::Mat<XF_32FC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAP_XL> mapxLMat(height, width);

        xf::cv::Mat<XF_32FC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAP_YL> mapyLMat(height, width);

        xf::cv::Mat<XF_32FC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAP_XR> mapxRMat(height, width);

        xf::cv::Mat<XF_32FC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAP_YR> mapyRMat(height, width);

        xf::cv::Mat<XF_8UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_LEFT_REMAPPED> leftRemappedMat(height, width);

        xf::cv::Mat<XF_8UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_RIGHT_REMAPPED> rightRemappedMat(height, width);

        xf::cv::Mat<OUT_TYPE, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAT_DISP> imgOutputStereo8(height, width);


    #pragma HLS DATAFLOW

        xf::cv::Array2xfMat<PTR_IN_WIDTH, XF_8UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAT_L>(img_L, mat_L);
        xf::cv::Array2xfMat<PTR_IN_WIDTH, XF_8UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAT_R>(img_R, mat_R);

        // Generate undistort + rectify maps for the left camera
        xf::cv::InitUndistortRectifyMapInverse<
                CAMERA_MATRIX_SIZE,       // Number of elements in the camera matrix 
                DIST_COEFF_SIZE,          // Number of distortion coefficients (commonly 5 or 8)
                XF_32FC1,                 // Pixel type for the maps (32-bit float, 1 channel)
                IMG_HEIGHT, IMG_WIDTH,   // Image height and width
                XF_NPPC1,                 // Pixels processed per clock (typically 1 for full precision)
                XF_CV_DEPTH_MAP_XL,       // HLS buffer depth for mapx (left)
                XF_CV_DEPTH_MAP_YL        // HLS buffer depth for mapy (left)
        >(
                cameraMA_l_fix, // [Input] Left camera intrinsic matrix (fixed-point or float)
                distC_l_fix,    // [Input] Left distortion coefficients
                irA_l_fix,      // [Input] Left rectification transformation matrix
                mapxLMat,       // [Output] Map for X-coordinates (left)
                mapyLMat,       // [Output] Map for Y-coordinates (left)
                CAMERA_MATRIX_SIZE,       // [Input] Size of the camera matrix (should be 3x3 = 9)
                DIST_COEFF_SIZE        // [Input] Number of distortion coefficients
        );

        // Apply the remap to rectify the left image
        xf::cv::remap<
                XF_REMAP_BUFSIZE,             // Optional tuning parameter for remap buffer size
                XF_INTERPOLATION_BILINEAR,    // Interpolation type for remapping (bilinear)
                XF_8UC1, XF_32FC1, XF_8UC1,   // Input image type, map type, output image type
                IMG_HEIGHT, IMG_WIDTH,        // Image dimensions
                XF_NPPC1,                     // Pixels per clock
                XF_USE_URAM,                  // Use UltraRAM if available (true/false)
                XF_CV_DEPTH_MAT_L,            // Buffer depth for input image (left)
                XF_CV_DEPTH_LEFT_REMAPPED,    // Buffer depth for output rectified image (left)
                XF_CV_DEPTH_MAP_XL, XF_CV_DEPTH_MAP_YL // Buffer depths for X and Y maps
        >(
                mat_L,            // [Input] Raw left camera image (grayscale)
                leftRemappedMat,  // [Output] Rectified left image
                mapxLMat,         // [Input] X-coordinate remap
                mapyLMat          // [Input] Y-coordinate remap
        );

        // Generate undistort + rectify maps for the right camera
        xf::cv::InitUndistortRectifyMapInverse<
                CAMERA_MATRIX_SIZE, DIST_COEFF_SIZE, XF_32FC1,
                IMG_HEIGHT, IMG_WIDTH, XF_NPPC1,
                XF_CV_DEPTH_MAP_XR, XF_CV_DEPTH_MAP_YR
        >(
                cameraMA_r_fix, // [Input] Right camera intrinsic matrix
                distC_r_fix,    // [Input] Right distortion coefficients
                irA_r_fix,      // [Input] Right rectification matrix
                mapxRMat,       // [Output] Map for X-coordinates (right)
                mapyRMat,       // [Output] Map for Y-coordinates (right)
                CAMERA_MATRIX_SIZE,       // [Input] Size of camera matrix
                DIST_COEFF_SIZE        // [Input] Size of distortion coefficients
        );

        // Apply the remap to rectify the right image
        xf::cv::remap<
                XF_REMAP_BUFSIZE, XF_INTERPOLATION_BILINEAR,
                XF_8UC1, XF_32FC1, XF_8UC1,
                IMG_HEIGHT, IMG_WIDTH,
                XF_NPPC1, XF_USE_URAM,
                XF_CV_DEPTH_MAT_R, XF_CV_DEPTH_RIGHT_REMAPPED,
                XF_CV_DEPTH_MAP_XR, XF_CV_DEPTH_MAP_YR
        >(
                mat_R,             // [Input] Raw right camera image (grayscale)
                rightRemappedMat,  // [Output] Rectified right image
                mapxRMat,          // [Input] X remap matrix
                mapyRMat           // [Input] Y remap matrix
        );

        // Compute disparity map using Stereo Block Matching
        xf::cv::StereoBM<
                SAD_WINDOW_SIZE,          // Size of the block window used for matching (e.g., 5, 7, 15)
                NO_OF_DISPARITIES,        // Number of disparity levels (must be divisible by 16)
                PARALLEL_UNITS,           // Level of parallelism (how many pixels are computed in parallel)
                XF_8UC1, XF_16UC1,         // Input image type (8-bit), output type (16-bit disparity)
                IMG_HEIGHT, IMG_WIDTH,    // Image dimensions
                XF_NPPC1, XF_USE_URAM,
                XF_CV_DEPTH_LEFT_REMAPPED,  // Buffer depth for left rectified image
                XF_CV_DEPTH_RIGHT_REMAPPED, // Buffer depth for right rectified image
                XF_CV_DEPTH_MAT_DISP        // Buffer depth for output disparity map
        >(
                leftRemappedMat,   // [Input] Rectified left image
                rightRemappedMat,  // [Input] Rectified right image
                mat_disp,          // [Output] Disparity map (scaled, typically 4x)
                bm_state           // [Input] Block matching configuration/state object
        );


        xf::cv::convertTo<XF_16UC1,OUT_TYPE, IMG_HEIGHT, IMG_WIDTH, NPC>(mat_disp,imgOutputStereo8,XF_CONVERT_16U_TO_8U,1);
                                                                        
        xf::cv::xfMat2Array<PTR_OUT_WIDTH, XF_8UC1, IMG_HEIGHT, IMG_WIDTH, XF_NPPC1, XF_CV_DEPTH_MAT_DISP>(imgOutputStereo8, img_disp);
    }