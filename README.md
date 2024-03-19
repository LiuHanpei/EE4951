Add your conv functions to the 'void dw_conv2d3x3' and 'void conv2d1x1' sections in the 'dw_conv3x3.cpp' and 'conv1x1.cpp' files. Look for the commented sections labeled "add your code here." This setup allows for the verification of your functions' outputs.

To run this code, you should first run the 'dw_conv3x3.py' and 'conv1x1.py' to generate the input, weight, bias and output binary files. Alternatively, you can also download them from here https://drive.google.com/drive/folders/1nbImh_oOa_hebjPtRWhmXfPHshYQs8Kn?usp=drive_link and add them to your directory.

The c++ code will first read in the bin files, and run the two functions to compare with your results.
The two functions are:
The 'void dw_conv2d3x3' ('void conv2d1x1') should be changed to your function.
For the 'void dw_conv2d3x3' and 'void conv2d1x1' functions in the 'dw_conv3x3.cpp' and 'conv1x1.cpp' files,
the inputs and the outputs have already been defined, please do not modify them, also, please do not modify the rest part of the codes.
The "void validateOutput" will verify your results.

If the results match, you should see a "PASSED".
If not, you will see the mismatching elements in your output.
3/19/2024
