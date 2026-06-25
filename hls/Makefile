# Get the current working directory
WORKSPACE := $(shell pwd)

# Paths to TCL script and arguments
TCL_SCRIPT 	:= $(WORKSPACE)/open_project.tcl
FUNCTION 	:= disparity
KERNEL 		:= disparity_kernel

export OPENCV_INCLUDE:=$(WORKSPACE)/opencv/install/include/opencv4 
export OPENCV_LIB:=$(WORKSPACE)/opencv/install/lib
export LD_LIBRARY_PATH:=$(WORKSPACE)/opencv/install/lib:$$LD_LIBRARY_PATH

.PHONY: info
info:
	@echo "Workspace:     $(WORKSPACE)"

.PHONY: setup
setup:
	@echo "Checking Vitis HLS..."
	@if ! command -v vitis_hls >/dev/null 2>&1; then \
		echo "Error: 'vitis_hls' is not available in PATH. Please source Vitis HLS setup script first."; \
		exit 1; \
	fi
	@echo "Vitis HLS is available."
	@echo "Creating directories..."
	@mkdir -p vitis_$(FUNCTION)
	@echo "Exporting opencv libary paths" 
	@echo "Running TCL script with parameters:"
	@echo " - $(WORKSPACE)"
	@echo " - $(FUNCTION)"
	@echo " - $(KERNEL)"
	@vitis_hls -f $(TCL_SCRIPT) -tclargs "$(WORKSPACE)" "$(FUNCTION)" "$(KERNEL)"