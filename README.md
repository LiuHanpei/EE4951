Add your conv functions to the "void dw_conv2d3x3" and "void conv2d1x1" area in the dw_conv3x3.cpp and conv1x1.cpp files, where you can see the commented "add your code here"
This code can be used to verify the outputs of your functions

To run this code,
First run the dw_conv3x3.py and conv1x1.py to generate the input, weight, bias and output binary files (optional, since they are already been generated)

The c++ code will first read in the bin files, and run the two functions to compare with your results.
The two functions are:
The "void dw_conv2d3x3"  ("void conv2d1x1") should be changed to your function
For the "void dw_conv2d3x3" and "void conv2d1x1" functions in the dw_conv3x3.cpp and conv1x1.cpp files,
The inputs and the outputs have already been defined, please do not modify them,
also, please do not modify the rest part of the codes.
The "void validateOutput" will verify your results

If the results match, you should see a "PASSED"
If not, you will see the mismatching elements in your output.
3/19/2024