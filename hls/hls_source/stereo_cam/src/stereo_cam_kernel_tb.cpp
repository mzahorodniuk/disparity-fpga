#include "common/xf_headers.hpp"
//#include "xf_stereolbm_config.h"
#include "stereo_cam_kernel.h"
#include "stereo_cam_config.h"

using namespace std;

typedef struct {
    unsigned int preFilterSize;     // Must be odd: 5, 7, 9, or 11
    unsigned int preFilterCap;      // Cap value for prefiltering (e.g., 31)
    unsigned int SADWindowSize;     // Size of the block window (e.g., 5, 9, 15)
    unsigned int minDisparity;      // Minimum disparity value (often 0)
    unsigned int numberOfDisparities; // Disparity range (must be multiple of 16)
    unsigned int textureThreshold;    // Filter weak textured areas (e.g., 1000)
    unsigned int uniquenessRatio;      // Ratio to filter ambiguous matches (e.g., 15)
    unsigned int ndisp_unit;           // Disparity step (usually 16)
    unsigned int sweepFactor;          // Platform-specific control (often 1)
    unsigned int remainder;            // Platform-specific padding/alignment (often 0)
} bm_state_t;

typedef union {
    bm_state_t param;
    unsigned int arr[10];
}bm_state_un;

int main(int argc, char** argv) {
    cv::setUseOptimized(false);

    std::cout << "Load parameters ....";

    cv::Mat K1 = (cv::Mat_<float>(3, 3) << 
        LEFT_CAM_FX, 0, LEFT_CAM_CX,
        0, LEFT_CAM_FY, LEFT_CAM_CY,
        0, 0, 1);

    cv::Mat K2 = (cv::Mat_<float>(3, 3) << 
        RIGHT_CAM_FX, 0, RIGHT_CAM_CX,
        0, RIGHT_CAM_FY, RIGHT_CAM_CY,
        0, 0, 1);

    cv::Mat D1 = (cv::Mat_<float>(8, 1) << 
        LEFT_CAM_K1, LEFT_CAM_K2, LEFT_CAM_P1, LEFT_CAM_P2, LEFT_CAM_K3, 0, 0 ,0);

    cv::Mat D2 = (cv::Mat_<float>(8, 1) << 
        RIGHT_CAM_K1, RIGHT_CAM_K2, RIGHT_CAM_P1, RIGHT_CAM_P2, RIGHT_CAM_K3, 0, 0 ,0);

    // Rotation vector to rotation matrix
    cv::Mat rvec = (cv::Mat_<float>(3, 1) << STEREO_RX_VGA, STEREO_CV_VGA, STEREO_RZ_VGA);
    cv::Mat R;
    cv::Rodrigues(rvec, R);

    // Translation vector (convert mm to meters if needed)
    cv::Mat T = (cv::Mat_<float>(3, 1) << STEREO_BASELINE, STEREO_TY, STEREO_TZ);
    std::cout << "R = \n" << R << std::endl;
    std::cout << "T = \n" << T << std::endl;
    cv::Mat R1, R2, P1, P2, Q;
    cv::Size imageSize(IMG_WIDTH, IMG_HEIGHT); // VGA

    std::cout << "Done" << std::endl;

    std::cout << "Calculate Rectify matrices ...";

    cv::stereoRectify(K1, D1, K2, D2, imageSize, R, T, R1, R2, P1, P2, Q);

    std::cout << "R1 = \n" << R1 << std::endl;
    std::cout << "R2 = \n" << R2 << std::endl;

    cv::Mat map1x, map1y, map2x, map2y;

    cv::initUndistortRectifyMap(K1, D1, R1, P1, imageSize, CV_16SC2, map1x, map1y);
    cv::initUndistortRectifyMap(K2, D2, R2, P2, imageSize, CV_16SC2, map2x, map2y);

    std::cout << "Done" << std::endl;

    std::cout << "Read images ...";

    // Reading in the images: Only Grayscale image
    cv::Mat imgL_raw = cv::imread("/home/sudent/Documents/umd/disparity_backup/disparity/data/left.jpg", cv::IMREAD_GRAYSCALE);
    cv::Mat imgR_raw = cv::imread("/home/sudent/Documents/umd/disparity_backup/disparity/data/right.jpg", cv::IMREAD_GRAYSCALE);

    if (imgL_raw.empty() || imgR_raw.empty()) {
        std::cerr << "Could not open one of the images!" << std::endl;
        return -1;
    }

    std::cout << "Done" << std::endl;

    std::cout << "Remap images ...";

    cv::Mat imgL, imgR;
    cv::remap(imgL_raw, imgL, map1x, map1y, cv::INTER_LINEAR);
    cv::remap(imgR_raw, imgR, map2x, map2y, cv::INTER_LINEAR);

    std::cout << "Done" << std::endl;

    std::cout << "Set up disparity opencv ...";

    cv::Ptr<cv::StereoBM> stereobm = cv::StereoBM::create(NO_OF_DISPARITIES, SAD_WINDOW_SIZE);
/*    
#                         "speckle_size": 7,
#                         "speckle_range": 30, 
#                         "correlation_window_size": 15, 
#                         "prefilter_cap": 21,
#                         "prefilter_size": 5, 
#                         "texture_threshold": 1000,
#                         "uniqueness_ratio": 15.0,
#                         "min_disparity": 0,
*/

        // Set parameters
    stereobm->setSpeckleWindowSize(7);               // speckle_size
    stereobm->setSpeckleRange(30);                   // speckle_range
    stereobm->setPreFilterCap(21);                   // prefilter_cap
    stereobm->setPreFilterSize(5);                   // prefilter_size (must be odd and in [5..255])
    stereobm->setTextureThreshold(1000);             // texture_threshold
    stereobm->setUniquenessRatio(15);                // uniqueness_ratio
    stereobm->setMinDisparity(0);                    // min_disparity

    std::cout << "Done" << std::endl;

    std::cout << "Calculate disparity opencv ...";

    // Compute disparity map
    cv::Mat disparity, hls_disp8;
    stereobm->compute(imgL, imgR, disparity);

    std::cout << "Done" << std::endl;

    std::cout << "Set up disparity hls ...";

    // Creating host memory for the hw acceleration
    hls_disp8.create(IMG_HEIGHT, IMG_WIDTH, CV_8UC1);

    // OpenCL section:
    bm_state_un bm;
    
    // Assign values manually
    bm.param.preFilterSize     = 5;
    bm.param.preFilterCap      = 21;
    bm.param.SADWindowSize     = 15;
    bm.param.minDisparity      = 0;
    bm.param.numberOfDisparities = NO_OF_DISPARITIES;
    bm.param.textureThreshold  = 20;
    bm.param.uniquenessRatio   = 15;
    bm.param.ndisp_unit        = 16;
    bm.param.sweepFactor       = 1;
    bm.param.remainder         = 0;


    std::cout << "Done" << std::endl;
    
    std::cout << "Calculate disparity hls ...";

    if (imgL.empty() || imgR.empty() || hls_disp8.empty()) {
        std::cerr << "Input images are empty!" << std::endl;
        return -1;
    }

    float cameraMA_l[9] = {LEFT_CAM_FX, 0, LEFT_CAM_CX, 0, LEFT_CAM_FY, LEFT_CAM_CY, 0, 0, 1};
    float cameraMA_r[9] = {RIGHT_CAM_FX, 0, RIGHT_CAM_CX, 0, RIGHT_CAM_FY, RIGHT_CAM_CY, 0, 0, 1};
    float distC_l[8] = {LEFT_CAM_K1, LEFT_CAM_K2, LEFT_CAM_P1, LEFT_CAM_P2, LEFT_CAM_K3, 0, 0, 0};
    float distC_r[8] = {RIGHT_CAM_K1, RIGHT_CAM_K2, RIGHT_CAM_P1, RIGHT_CAM_P2, RIGHT_CAM_K3, 0, 0, 0};

    cv::Mat R1f, R2f;
    R1.convertTo(R1f, CV_32F);
    R2.convertTo(R2f, CV_32F);
    cv::Mat flat1 = R1f.reshape(1, 1);
    cv::Mat flat2 = R2f.reshape(1, 1);

    std::cout << "flat1 = \n" << flat1 << std::endl;
    std::cout << "flat2 = \n" << flat2 << std::endl;

    int totalElements1 = flat1.total();
    int totalElements2 = flat2.total();
    float irA_l[totalElements1];
    float irA_r[totalElements2];
    for(int i = 0; i < totalElements1; i++)
        irA_l[i] = flat1.ptr<float>()[i];
    for(int i = 0; i < totalElements2; i++)
        irA_r[i] = flat2.ptr<float>()[i];
    

    std::cout << std::endl;
    for(int i = 0; i < totalElements1; i++)
        std::cout << irA_l[i] << "|";
    std::cout << std::endl;

    for(int i = 0; i < totalElements2; i++)
        std::cout << irA_r[i]<< "|";
    std::cout << std::endl;


    stereo_cam_kernel(
        (ap_uint<PTR_IN_WIDTH>*) imgL_raw.data,
        (ap_uint<PTR_IN_WIDTH>*) imgR_raw.data,
        (ap_uint<PTR_OUT_WIDTH>*) hls_disp8.data,
        cameraMA_l,
        cameraMA_r,
        distC_l,
        distC_r,
        irA_l,
        irA_r,
        bm.arr,
        IMG_HEIGHT, IMG_WIDTH
    );

    std::cout << "Done" << std::endl;

    std::cout << "Calculate rest...";

    // Normalize the disparity map for visualization
    cv::Mat disp8U;

    disparity.convertTo(disp8U, CV_8U, 255 / (NO_OF_DISPARITIES * 16.0));

    // Reproject to 3D
    cv::Mat points3D_cv, points3D_hls;


    cv::Mat distance,distance_hls;
    cv::Mat disp8U_f, hls_disp8_f;

    disp8U.convertTo(disp8U_f, CV_32F);
    hls_disp8.convertTo(hls_disp8_f, CV_32F); 
    
    cv::divide(STEREO_BASELINE/1000.0 * LEFT_CAM_FX * 255.0 / NO_OF_DISPARITIES, disp8U_f, distance, 1.0, CV_32F);
    cv::divide(STEREO_BASELINE/1000.0 * LEFT_CAM_FX * 255.0 / NO_OF_DISPARITIES, hls_disp8_f, distance_hls, 1.0, CV_32F);

    reprojectImageTo3D(distance, points3D_cv, Q, true); // handleMissingValues = true
    reprojectImageTo3D(distance_hls, points3D_hls, Q, true); // handleMissingValues = true

    // Now points3D.at<Vec3f>(y,x) gives you the 3D (X, Y, Z) in real-world space
    // Z is the distance from the camera

    std::vector<cv::Point> pointsToCheck = {
        {200, 200}, {250, 280}, {420, 250} // Add as many as you like
    };

    cv::Mat img_color_cv, img_color_hls;

    cv::cvtColor(disp8U, img_color_cv, cv::COLOR_GRAY2BGR);
    cv::cvtColor(hls_disp8, img_color_hls, cv::COLOR_GRAY2BGR);
    

    for (size_t i = 0; i < pointsToCheck.size(); i++) {
        const cv::Point& pt = pointsToCheck[i];

        //cv::Vec3f point_cv = distance.at(pt);
        auto distance_cv_p = (float)distance.at<float>(pt);

        //cv::Vec3f point_hls = distance_hls.at(pt);
        auto distance_hls_p = (float)distance_hls.at<float>(pt);

        // Draw red circles
        cv::circle(img_color_cv, pt, 5, cv::Scalar(0, 0, 255), 2);
        cv::circle(img_color_hls, pt, 5, cv::Scalar(0, 0, 255), 2);

        // Show distances
        char text[100];
        snprintf(text, sizeof(text), "%.2f m", distance_cv_p);
        cv::putText(img_color_cv, text, pt + cv::Point(10, -10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);

        snprintf(text, sizeof(text), "%.2f m", distance_hls_p);
        cv::putText(img_color_hls, text, pt + cv::Point(10, -10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 1);
    }
    std::cout << "Done" << std::endl;

    std::cout << "Save images...";
    
    cv::imwrite("/home/sudent/Documents/umd/disparity_backup/disparity/sim_output/disp_img_cv.jpg", disp8U);
    cv::imwrite("/home/sudent/Documents/umd/disparity_backup/disparity/sim_output/disp_img_hls.jpg", hls_disp8);
    cv::imwrite("/home/sudent/Documents/umd/disparity_backup/disparity/sim_output/disp_dot_cv.jpg", img_color_cv);
    cv::imwrite("/home/sudent/Documents/umd/disparity_backup/disparity/sim_output/disp_dot_hls.jpg", img_color_hls);

    std::cout << "Done" << std::endl;

    return 0;
}