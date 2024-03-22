#define batchSize 4
#define numInputChannels 3
#define numOutputChannels 3
#define stride 1
#define kernelSize 3
#define inputHeight 160
#define inputWidth 320
#define outputHeight 160 // adjust it if you change other parameters
#define outputWidth 320 // adjust it if you change other parameters
#define EPS 1e-5
#include <iostream>
#include <fstream>
#include <stdlib.h>

void dw_conv2d3x3(float inBuffer[batchSize][numInputChannels][inputHeight][inputWidth],
    float wBuffer[numOutputChannels][numInputChannels][kernelSize][kernelSize],
    float biasBuffer[numOutputChannels],
    float outBuffer[batchSize][numOutputChannels][outputHeight][outputWidth]){


    //add your code here
    //if the result is correct, you should see a 'PASSED'
}


void validateOutput(float out[batchSize][numOutputChannels][outputHeight][outputWidth],
    float outGolden[batchSize][numOutputChannels][outputHeight][outputWidth]){
    for (int b = 0; b < batchSize; b++){
        for (int co = 0; co < numOutputChannels; co++){
            for (int h = 0; h < outputHeight; h++){
                for (int w = 0; w < outputWidth; w++){
                    if (abs(outGolden[b][co][h][w] - out[b][co][h][w]) > EPS ){
                        std::cout << "output[" << b << ","<< co << "," << h << "," << w << "]: " << outGolden[b][co][h][w] << " != " << out[b][co][h][w] << std::endl;
                        return;
                    }
                }
            }
        }
    }
    std::cout << "PASSED" << std::endl;
    return;
}

int main(){
    float inBuffer[batchSize][numInputChannels][inputHeight][inputWidth];
    float wBuffer[numOutputChannels][numInputChannels][kernelSize][kernelSize];
    float biasBuffer[numOutputChannels];
    float outBuffer[batchSize][numOutputChannels][outputHeight][outputWidth];
    float outBufferGolden[batchSize][numOutputChannels][outputHeight][outputWidth];
    
    std::ifstream ifs_in("dw_conv_input3x3.bin", std::ios::in | std::ios::binary);
    if (!ifs_in.is_open()) {
        std::cerr << "Failed to open input.bin" << std::endl;
        return 1; // or other error handling
    }
    ifs_in.read((char*)(***inBuffer), batchSize * numInputChannels * inputHeight * inputWidth * sizeof(float));
    
    std::ifstream ifs_param("dw_conv_weight3x3.bin", std::ios::in | std::ios::binary);
    ifs_param.read((char*)(***wBuffer), numOutputChannels * numInputChannels * kernelSize * kernelSize * sizeof(float));
    
    std::ifstream ifs_bias("dw_conv_bias3x3.bin", std::ios::in | std::ios::binary);
    ifs_bias.read((char*)(biasBuffer), numOutputChannels * sizeof(float));
    
    dw_conv2d3x3(inBuffer, wBuffer, biasBuffer, outBuffer);    
    // std::ofstream ofs_out("output_c.bin", std::ios::out | std::ios::binary);
    // ofs_out.write((char*)(***outBuffer), batchSize * numOutputChannels * outputHeight * outputWidth * sizeof(float) );
    std::ifstream ofs_out_golden("dw_conv_output_torch3x3.bin", std::ios::out | std::ios::binary);
    ofs_out_golden.read((char*)(***outBufferGolden), batchSize * numOutputChannels * outputHeight * outputWidth * sizeof(float) );
    validateOutput(outBuffer, outBufferGolden);
}
