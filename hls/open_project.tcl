# ---------------------------------------------------
# Project Setup
# ---------------------------------------------------
# Check argument count
if { $argc != 5 } {
    puts "Error: Expected 5 arguments, but got $argv"
    exit 1
}

set PROJ_ROOT   [lindex $argv 2]
set FUNCT       [lindex $argv 3]
set ACC_KERNEL  [lindex $argv 4]


set PROJ    "vitis_${FUNCT}"
set SOLN    "sol1"
set TOP     $ACC_KERNEL
set PART    "xck26-sfvc784-2LV-c"

# Set default clock period if not already defined
if {![info exists CLKP]} {
    set CLKP 3.3
}

# ---------------------------------------------------
# Open Project
# ---------------------------------------------------
open_project -reset $PROJ

# ---------------------------------------------------
# Add Design Files
# ---------------------------------------------------
set DESIGN_SRC "${PROJ_ROOT}/hls_source/${FUNCT}/src/${ACC_KERNEL}.cpp"

set DESIGN_CFLAGS "-I ${PROJ_ROOT}/Vitis_Libraries/vision/L1/include \
                   -I ${PROJ_ROOT}/hls_source/${FUNCT}/include \
                   -I ./ \
                   -D__SDSVHLS__ -std=c++0x"


add_files $DESIGN_SRC -cflags "$DESIGN_CFLAGS" -csimflags ""


# ---------------------------------------------------
# Add Testbench Files
# ---------------------------------------------------
set TB_SRC "${PROJ_ROOT}/hls_source/${FUNCT}/src/${ACC_KERNEL}_tb.cpp"
set TB_CFLAGS "-I ${PROJ_ROOT}/opencv/install/include/opencv4 \
               -I ${PROJ_ROOT}/opencv/install/lib \
               -I ${PROJ_ROOT}/opencv/install \
               -I ${PROJ_ROOT}/Vitis_Libraries/vision/L1/include \
               -I ./ \
               -D__SDSVHLS__ -std=c++0x"

add_files -tb $TB_SRC -cflags "$TB_CFLAGS" -csimflags ""

# ---------------------------------------------------
# Solution Setup
# ---------------------------------------------------
set_top $TOP
open_solution -reset $SOLN

set_part $PART
create_clock -period $CLKP

set OPENCV_LIB "${PROJ_ROOT}/opencv/install/lib"

csim_design -ldflags "-L $OPENCV_LIB -lopencv_imgcodecs -lopencv_imgproc -lopencv_calib3d -lopencv_core -lopencv_highgui -lopencv_flann -lopencv_features2d"
csynth_design
export_design -format ip_catalog -rtl verilog
