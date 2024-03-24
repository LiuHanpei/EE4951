This code is designed to validate the dw_conv3x3 function and conv1x1 function. Please incorporate your function into the 'dw_conv3x3.cpp' file and 'conv1x1.cpp' file.

In the testbench file named 'sim.cpp', you will encounter a preprocessor directive #define PRINT_DEBUG. This directive enables the printing of both expected values and your function's results. Additionally, you can specify the indices you wish to print at https://github.com/LiuHanpei/EE4951/blob/b4f58940206070863843f28a539bbef6d075b3c5/milestone2_hls_verify/EE4951_hls_veridw_3x3/sim.cpp#L132-L133 and https://github.com/LiuHanpei/EE4951/blob/77fd6510f29caf21a6a3c2d380f7e4a59973eb90/milestone2_hls_verify/EE4951_hls_veri_1x1/sim.cpp#L125-L126 respectively.

Within the 'conv.h' file, you'll find "#define CSIM_DEBUG". This enables the simulation of floating-point results. If the Mean Squared Error (MSE) is greater than 0 and less than 10^-13, a message stating "Floating-point Simulation SUCCESSFUL!!!" will be displayed. Conversely, if the MSE does not fall within these parameters, the message "Floating-point Simulation FAILED :(" will appear.

To simulate fixed-point results, simply comment out the "#define CSIM_DEBUG". Should the MSE be greater than 0 and less than 10^-3, a message indicating "Fixed-point Simulation SUCCESSFUL!!!" will be shown. If not, you will see "Fixed-point Simulation FAILED :(".
