#include "utils.h"

void load_input_from_DRAM (
    fm_t in_fm_buf[IN_DEPTH][IN_HEIGHT][IN_WIDTH],
    fm_t in_fm[IN_DEPTH][IN_HEIGHT][IN_WIDTH]
)
{
    const int P = PADDING;

	INPUT_BUFFER_DEPTH:
	for(int c = 0; c < IN_DEPTH; c++)
	{
		INPUT_BUFFER_HEIGHT:
		for (int i = 0; i < IN_HEIGHT; i++)
		{
		INPUT_BUFFER_WIDTH:
			for (int j = 0; j < IN_WIDTH; j++)
			{
				// Handle border features here

					in_fm_buf[c][i][j] = in_fm[c][i][j];
			}
		}

	}
}

void load_weight_bias_from_DRAM (
    wt_t weight_buf[OUT_DEPTH][IN_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH],
    wt_t bias_buf[OUT_DEPTH],
    wt_t weights[OUT_DEPTH][IN_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH],
    wt_t bias[OUT_DEPTH]
)
{

    WEIGHT_KERNEL_NUM:
    for(int f = 0; f < OUT_DEPTH; f++)
    {
        WEIGHT_KERNEL_DEPTH:
        for(int c = 0; c < IN_DEPTH; c++)
        {
            WEIGHT_KERNEL_HEIGHT:
            for(int kh = 0; kh < KERNEL_HEIGHT; kh++)
	        {
                WEIGHT_KERNEL_WIDTH:
	            for(int kw = 0; kw < KERNEL_WIDTH; kw++)
	            {
	                weight_buf[f][c][kh][kw] = weights[f][c][kh][kw];
                }
            }
        }
    }

    BIAS:
    for(int f = 0; f < OUT_DEPTH; f++)
    {
        bias_buf[f] = bias[f];
    }
}

void store_output_tile_to_DRAM (
    fm_t out_fm[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH],
    fm_t out_fm_buf[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH]
)
{


	OUTPUT_BUFFER_DEPTH:
	for(int f = 0; f < OUT_DEPTH; f++)
	{
		OUTPUT_BUFFER_HEIGHT:
		for(int i = 0; i < OUT_HEIGHT; i++)
		{
			OUTPUT_BUFFER_WIDTH:
			for(int j = 0; j < OUT_WIDTH; j++)
			{
				out_fm[f][i][j] = out_fm_buf[f][i][j];
			}
		}
	}

}
