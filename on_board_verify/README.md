This guide outlines how to test the functionality of your design on the Ultra96V2 board. To access the board interface, navigate to http://192.168.3.1/ , and locate the "EE4951_on_board_verify" file for the code essential for verification. The name of the code is “control_code.ipynb”.

Before executing the bitstream, ensure to upload your binary files along with the ".bit" and ".hwh" files related to your design.

It's important to adjust the parameters and specify the name of your design, along with the address if required, to match your project specifics.

For detailed instructions on file locations, input and output addresses, and additional setup information, refer to the tutorial available at https://github.com/yangkatiezhao/HLS-Tutorial-Vivado2018.2-on-VLSI-Lab-Machines/blob/main/README.md

Upon successfully running the code, the interface will display the time latency and Mean Squared Error (MSE) results, allowing you to evaluate the performance of your design directly on the board.

The binary file for testing is available at https://drive.google.com/drive/folders/1uQe7YoCl0NhBBnAQBZjp3BPEYlbIJW-X?usp=sharing, you can also use your own binary files for testing