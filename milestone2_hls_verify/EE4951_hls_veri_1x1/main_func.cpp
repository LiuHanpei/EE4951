#include "utils.h"



void main_func (
    fm_t input_feature_map[IN_DEPTH][IN_HEIGHT][IN_WIDTH],
    wt_t layer_weights[OUT_DEPTH][IN_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH],
    wt_t layer_bias[OUT_DEPTH],
    fm_t output_feature_map[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH]
)
{
    //--------------------------------------------------------------------------
    // Defines interface IO ports for HLS.
    //--------------------------------------------------------------------------
    #pragma HLS INTERFACE m_axi depth=1  port=input_feature_map   bundle=fm
    #pragma HLS INTERFACE m_axi depth=1  port=layer_weights       bundle=wt
    #pragma HLS INTERFACE m_axi depth=1  port=layer_bias          bundle=wt
    #pragma HLS INTERFACE m_axi depth=1  port=output_feature_map  bundle=fm
    #pragma HLS INTERFACE s_axilite register	port=return

    fm_t conv_in_buf[IN_DEPTH][IN_HEIGHT][IN_WIDTH];
    wt_t conv_wt_buf[OUT_DEPTH][IN_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH];
    wt_t conv_bias_buf[OUT_DEPTH];
    fm_t conv_out_buf[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH];

    //--------------------------------------------------------------------------
    // Process each tile iteratively
    //--------------------------------------------------------------------------
    LOAD_INPUT:
    load_input_from_DRAM(
        conv_in_buf,
        input_feature_map
    );


    LOAD_WEIGHT_BIAS:
    load_weight_bias_from_DRAM(
    	conv_wt_buf,
        conv_bias_buf,
        layer_weights,
        layer_bias
    );

    CONV1x1:
	conv_1x1(
		conv_out_buf,
		conv_in_buf,
		conv_wt_buf,
		conv_bias_buf
	);

	STORE_DRAM:
	store_output_tile_to_DRAM(
		output_feature_map,
		conv_out_buf
	);
}
