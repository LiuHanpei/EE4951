#ifndef CONV_H_
#define CONV_H_

#include <iostream>
#include <ap_fixed.h>
//--------------------------------------------------------------------------
// Select DEBUG MODE HERE�� fixed-point or floating-point
//--------------------------------------------------------------------------

//comment this line if you want to run fixed-point simulation
#define CSIM_DEBUG



//--------------------------------------------------------------------------
// Type conversions for simulation and synthesis
//--------------------------------------------------------------------------


#define IN_DEPTH       3
#define IN_HEIGHT      160
#define IN_WIDTH       320

#define OUT_DEPTH      3
#define OUT_HEIGHT     160
#define OUT_WIDTH      320

#define STRIDE            1
#define PADDING           1
#define KERNEL_HEIGHT     3
#define KERNEL_WIDTH      3
#define BATCHSIZE         1


#ifdef  CSIM_DEBUG
	typedef float fm_t;
	typedef float wt_t;
#else
	typedef ap_fixed<16,3> fm_t;
	typedef ap_fixed<16,3> wt_t;
#endif


//--------------------------------------------------------------------------
// Top-level Function Declaration
//--------------------------------------------------------------------------
void main_func
(
    fm_t input_feature_map[IN_DEPTH][IN_HEIGHT][IN_WIDTH],
    wt_t layer_weights[OUT_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH],
    wt_t layer_bias[OUT_DEPTH],
    fm_t output_feature_map[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH]
);

#endif
