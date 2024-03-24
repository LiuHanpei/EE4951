############################################################
## This file is generated automatically by Vitis HLS.
## Please DO NOT edit it.
## Copyright 1986-2022 Xilinx, Inc. All Rights Reserved.
## Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
############################################################
open_project EE4951_hls_veridw_3x3
set_top main_func
add_files EE4951_hls_veridw_3x3/conv.h
add_files EE4951_hls_veridw_3x3/dw_conv3x3.cpp
add_files EE4951_hls_veridw_3x3/dw_conv_bias3x3.bin
add_files EE4951_hls_veridw_3x3/dw_conv_input3x3.bin
add_files EE4951_hls_veridw_3x3/dw_conv_output_torch3x3.bin
add_files EE4951_hls_veridw_3x3/dw_conv_weight3x3.bin
add_files EE4951_hls_veridw_3x3/main_func.cpp
add_files EE4951_hls_veridw_3x3/utils.cpp
add_files EE4951_hls_veridw_3x3/utils.h
add_files -tb EE4951_hls_veridw_3x3/sim.cpp -cflags "-Wno-unknown-pragmas"
open_solution "solution1" -flow_target vivado
set_part {xczu9eg-ffvb1156-2-e}
create_clock -period 10 -name default
config_cosim -tool xsim
source "./EE4951_hls_veridw_3x3/solution1/directives.tcl"
csim_design
csynth_design
cosim_design -tool xsim
export_design -format ip_catalog
