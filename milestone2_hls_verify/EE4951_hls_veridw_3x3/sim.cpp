#include <iostream>
#include <fstream>
#include <cmath>
#include "conv.h"

using namespace std;


#define PRINT_DEBUG

float conv_layer_input_feature_map[IN_DEPTH][IN_HEIGHT][IN_WIDTH];
float conv_layer_weights1[OUT_DEPTH][IN_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH];
float conv_layer_weights[OUT_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH];
float conv_layer_bias[OUT_DEPTH];
float conv_layer_golden_output_feature_map[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH];

fm_t  fixp_conv_layer_input_feature_map[IN_DEPTH][IN_HEIGHT][IN_WIDTH];
wt_t  fixp_conv_layer_weights[OUT_DEPTH][KERNEL_HEIGHT][KERNEL_WIDTH];
wt_t  fixp_conv_layer_bias[OUT_DEPTH];
fm_t  fixp_conv_layer_output_feature_map[OUT_DEPTH][OUT_HEIGHT][OUT_WIDTH] = {0};

void read_bin_files()
{
    // Input Feature Map
    ifstream ifs_conv_input("dw_conv_input3x3.bin", ios::in | ios::binary);
    if (!ifs_conv_input.is_open()) {
           std::cerr << "Failed to open input.bin" << std::endl;
           return ; // or other error handling
       }
    ifs_conv_input.read((char*)(**conv_layer_input_feature_map), IN_DEPTH*IN_HEIGHT*IN_WIDTH*sizeof(float));
    ifs_conv_input.close();

    // Typecast to fixed-point

	for(int c = 0; c < IN_DEPTH; c++)
		for(int i = 0; i < IN_HEIGHT; i++)
			for(int j = 0; j < IN_WIDTH; j++){
				fixp_conv_layer_input_feature_map[c][i][j] = (fm_t) conv_layer_input_feature_map[c][i][j];
				//std::cout << conv_layer_input_feature_map[c][i][j] << std::endl;
			}

    // Weights
    ifstream ifs_conv_weights("dw_conv_weight3x3.bin", ios::in | ios::binary);
    if (!ifs_conv_weights.is_open()) {
           std::cerr << "Failed to open weight.bin" << std::endl;
           return ; // or other error handling
       }
    ifs_conv_weights.read((char*)(***conv_layer_weights1), OUT_DEPTH*IN_DEPTH*KERNEL_HEIGHT*KERNEL_WIDTH*sizeof(float));
    ifs_conv_weights.close();

    for(int i = 0; i < OUT_DEPTH; i++)
    	for(int j = 0; j < KERNEL_HEIGHT; j++)
			for(int k = 0; k < KERNEL_WIDTH; k++)
				conv_layer_weights[i][j][k] = conv_layer_weights1[i][0][j][k];




    // Typecast to fixed-point
    for(int f = 0; f < OUT_DEPTH; f++)
        for(int m = 0; m < KERNEL_HEIGHT; m++)
			for(int n =0; n < KERNEL_WIDTH; n++)
				fixp_conv_layer_weights[f][m][n] = (wt_t) conv_layer_weights[f][m][n];

    // Bias
    ifstream ifs_conv_bias("dw_conv_bias3x3.bin", ios::in | ios::binary);
    if (!ifs_conv_bias.is_open()) {
           std::cerr << "Failed to open bias.bin" << std::endl;
           return ; // or other error handling
       }
    ifs_conv_bias.read((char*)(conv_layer_bias), OUT_WIDTH*sizeof(float));
    ifs_conv_bias.close();

    // Typecast to fixed-point
    for(int f = 0; f < OUT_DEPTH; f++)
        fixp_conv_layer_bias[f] = (wt_t) conv_layer_bias[f];

    // Golden Output
    ifstream ifs_golden_output("dw_conv_output_torch3x3.bin", ios::in | ios::binary);
    ifs_golden_output.read((char*)(**conv_layer_golden_output_feature_map), OUT_DEPTH * OUT_HEIGHT * OUT_WIDTH*sizeof(float));
    if (!ifs_golden_output.is_open()) {
           std::cerr << "Failed to open output.bin" << std::endl;
           return ; // or other error handling
       }
    ifs_golden_output.close();

	for(int c = 0; c < OUT_DEPTH; c++)
		for(int i = 0; i < OUT_HEIGHT; i++)
			for(int j = 0; j < OUT_WIDTH; j++)
			{
				fixp_conv_layer_input_feature_map[c][i][j] = (fm_t) conv_layer_input_feature_map[c][i][j];
				//std::cout << conv_layer_golden_output_feature_map[c][i][j] << std::endl;
			}
}

int main ()
{



    long double mse = 0.0;

    // Read reference inputs, parameters, and output
    read_bin_files();

    std::cout << "Beginning HLS  simulation..." << std::endl;

    main_func (fixp_conv_layer_input_feature_map,
                fixp_conv_layer_weights,
                fixp_conv_layer_bias,
                fixp_conv_layer_output_feature_map
    );

    std::cout << "simulation complete!\n" << std::endl;

    //Compute Mean-Squared-Error

	for(int f = 0; f < OUT_DEPTH; f++)
	{
		for(int i = 0; i < OUT_HEIGHT; i++)
		{
			for(int j = 0; j < OUT_WIDTH; j++)
			{
				mse += std::pow((conv_layer_golden_output_feature_map[f][i][j]
								 -(float) fixp_conv_layer_output_feature_map[f][i][j]), 2);
			}
		}

		#ifdef PRINT_DEBUG
			// Prints sample output values (first feature of each channel) for comparison
			// Modify as required for debugging
			int row = 15;
			int col = 15;
			cout << "Output feature[" << f << "][" << row << "][" << col << "]: ";
			cout << "Expected: " << conv_layer_golden_output_feature_map[f][row][col] << "\t";
			cout << "Actual: "   << fixp_conv_layer_output_feature_map[f][row][col];
			cout << endl;
		#endif
	}


    mse = mse / ( OUT_WIDTH * OUT_HEIGHT * OUT_WIDTH);

    std::cout << "\nOutput MSE:  " << mse << std::endl;

    std::cout << "----------------------------------------" << std::endl;



    #ifdef CSIM_DEBUG
        if(mse > 0 && mse < std::pow(10,-13))
        {
            std::cout << "Floating-point Simulation SUCCESSFUL!!!" << std::endl;
        }
        else
        {
            std::cout << "Floating-point Simulation FAILED :(" << std::endl;
        }
    #else
        if(mse > 0 && mse < std::pow(10,-3))
        {
            std::cout << "Fixed-point Simulation SUCCESSFUL!!!" << std::endl;
        }
        else
        {
            std::cout << "Fixed-point Simulation FAILED :(" << std::endl;
        }
    #endif
    std::cout << "----------------------------------------" << std::endl;

    return 0;
}
