#include "conv.h"

void load_input_from_DRAM (
    fm_t in_fm_buf[IN_DEPTH][IN_HEIGHT][IN_WIDTH],
    fm_t in_fm[IN_DEPTH][IN_HEIGHT][IN_WIDTH]

);

void load_weight_bias_from_DRAM (
    wt_t weight_buf[OUT_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH],
    wt_t bias_buf[OUT_DEPTH],
    wt_t weights[OUT_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH],
    wt_t bias[OUT_DEPTH]
);

void dw_conv_3x3 (
    fm_t Y_buf[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH],
    fm_t X_buf[IN_DEPTH][IN_HEIGHT][IN_WIDTH],
    wt_t W_buf[OUT_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH],
    wt_t B_buf[OUT_DEPTH]
);

void store_output_tile_to_DRAM (
    fm_t out_fm[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH],
    fm_t out_fm_buf[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH]
);
