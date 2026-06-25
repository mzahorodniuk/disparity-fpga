# disparity-fpga

Stereo **disparity** acceleration for the AMD/Xilinx **Kria K26** (Zynq UltraScale+ MPSoC),
spanning the full hardware design: the Vitis HLS compute kernels and the Vivado
platform that integrates them.

The disparity kernel is written in HLS, packaged as an IP, and instantiated inside the
Vivado block design alongside a DPU (`DPUCZDX8G`). Because the kernel and the platform
are two halves of one design, they live in one repository so a single commit/tag always
captures a consistent, buildable snapshot.

```
disparity-fpga/
├── hls/                       # Vitis HLS workspace (compute kernels)
│   ├── hls_source/
│   │   ├── disparity/         #   disparity_kernel  (set_top target)
│   │   │   ├── include/
│   │   │   └── src/           #   disparity_kernel.cpp + _tb.cpp
│   │   └── stereo_cam/        #   stereo_cam_kernel  (rectification + capture)
│   │       ├── include/
│   │       └── src/
│   ├── data/                  #   left.jpg / right.jpg test pair
│   ├── Makefile               #   `make setup` → csim + csynth + export IP
│   ├── open_project.tcl       #   Vitis HLS project script
│   └── BUILD_NOTES.md         #   original build notes (OpenCV cmake, etc.)
│
└── vivado/
    └── base_plat/             # Vivado project (source only — generated dirs gitignored)
        ├── base_plat.xpr       #   open this in Vivado
        ├── base_plat.srcs/     #   top.bd block design, .xci IP, wrapper, .xdc
        ├── top_wrapper.xsa     #   exported hardware handoff (for PetaLinux / Vitis)
        └── archive_project_summary.txt
```

## Target

| | |
|---|---|
| Device | Kria K26 SOM — `xck26-sfvc784-2LV-c` |
| HLS clock | 3.3 ns (≈303 MHz) |
| Toolchain | Vitis HLS / Vivado **2022.2** |
| Platform | ZynqMP block design + `DPUCZDX8G` DPU + disparity kernel IP |

## 1. HLS kernels (`hls/`)

The HLS build depends on two large trees that are **not** committed (they are vendored
locally and gitignored). Place them inside `hls/` before building:

- `hls/opencv/install/` — OpenCV built from source (used by the C-sim testbench).
  See [`hls/BUILD_NOTES.md`](hls/BUILD_NOTES.md) for the exact `cmake` invocation.
- `hls/Vitis_Libraries/` — AMD Vitis Vision Library (matched to 2022.2):
  ```bash
  git clone --branch 2022.2 https://github.com/Xilinx/Vitis_Libraries.git hls/Vitis_Libraries
  ```

Then, from `hls/`, with Vitis HLS on your `PATH`:

```bash
cd hls
source <Vitis_HLS>/2022.2/settings64.sh   # provides vitis_hls
make setup                                 # runs open_project.tcl: csim → csynth → export IP
```

`make setup` runs C simulation against the `data/` stereo pair, synthesizes, and exports
the kernel as an IP catalog entry. The generated `vitis_disparity/` project is gitignored.

## 2. Vivado platform (`vivado/base_plat/`)

Open the project in Vivado 2022.2:

```bash
vivado vivado/base_plat/base_plat.xpr
```

Only design sources are tracked (block design `top.bd`, the `.xci` IP configurations,
the `top_wrapper.v` HDL wrapper, and `can_pins.xdc` constraints). Vivado regenerates
`*.cache/`, `*.runs/`, `*.gen/`, `*.hw/` on open/build — these are gitignored.

`top_wrapper.xsa` is the exported hardware handoff, included for the software side
(PetaLinux / Vitis application build).

> **Kernel ↔ platform coupling:** the platform instantiates the disparity kernel as
> `top_disparity_kernel_0_0`. If you change the kernel's interface in `hls/`, re-export
> the IP (step 1) and update the IP in the block design before re-running synthesis.

## Notes

- Vivado/HLS version is pinned to **2022.2**; opening the `.xpr` in a different major
  version will trigger an upgrade flow.
- This repository was reorganized from the original Vitis HLS working tree; the large
  vendor/generated artifacts were intentionally left out so the repo stays source-only.
