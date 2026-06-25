set_top my_top_function
add_files my_function.cpp
add_files -tb testbench.cpp
open_solution "solution1"
set_part {xc7z020clg400-1}
create_clock -period 10 -name default
csim_design
exit


cmake -D CMAKE_BUILD_TYPE=RELEASE \
  -D CMAKE_INSTALL_PREFIX=/home/kria/Documents/disparity/install \
  -D CMAKE_CXX_COMPILER=/home/kria/xil/Vitis_HLS/2022.2/tps/lnx64/gcc-6.2.0/bin/g++ \
  -D OPENCV_EXTRA_MODULES_PATH=/home/kria/Documents/disparity/source_contrib/modules/ \
  -D WITH_V4L=ON \
  -D BUILD_TESTS=OFF \
  -D BUILD_ZLIB=ON \
  -D BUILD_JPEG=ON \
  -D WITH_JPEG=ON \
  -D WITH_PNG=ON \
  -D BUILD_EXAMPLES=OFF \
  -D INSTALL_C_EXAMPLES=OFF \
  -D INSTALL_PYTHON_EXAMPLES=OFF \
  -D WITH_OPENEXR=OFF \
  -D BUILD_OPENEXR=OFF \
  /home/kria/Documents/disparity/source


if lib 11 nit found on ubuntu 22.04
sudo apt update
cd ~/Downloads
wget http://mirrors.kernel.org/ubuntu/pool/main/libi/libidn/libidn11_1.33-2.2ubuntu2_amd64.deb
sudo apt install ./libidn11_1.33-2.2ubuntu2_amd64.deb

vitis_hls -f open_project.tcl

export OPENCV_INCLUDE=/home/kria/Documents/disparity/opencv/install/include/opencv4
export OPENCV_LIB=/home/kria/Documents/disparity/opencv/install/lib
export LD_LIBRARY_PATH=/home/kria/Documents/disparity/opencv/install/lib:$LD_LIBRARY_PATH

csim_design -ldflags "-L ${OPENCV_LIB} -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_core -lopencv_highgui -lopencv_flann -lopencv_features2d"
