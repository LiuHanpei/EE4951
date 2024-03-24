############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_project EE4951_hls_veri_1x1
set_top main_func
add_files conv.h
add_files EE4951_hls_veri_1x1/conv1x1.cpp
add_files EE4951_hls_veri_1x1/conv_bias1x1.bin
add_files EE4951_hls_veri_1x1/conv_input1x1.bin
add_files EE4951_hls_veri_1x1/conv_output_torch1x1.bin
add_files EE4951_hls_veri_1x1/conv_weight1x1.bin
add_files EE4951_hls_veri_1x1/main_func.cpp
add_files EE4951_hls_veri_1x1/utils.cpp
add_files EE4951_hls_veri_1x1/utils.h
add_files -tb EE4951_hls_veri_1x1/sim.cpp -cflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xczu9eg-ffvb1156-2-e}
create_clock -period 10 -name default
config_cosim -tool xsim
source "./EE4951_hls_veri_1x1/solution1/directives.tcl"
csim_design
csynth_design
cosim_design -tool xsim
export_design -format ip_catalog
