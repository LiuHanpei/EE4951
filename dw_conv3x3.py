import torch
import struct

def saveTensor2Bin(tensor, fileName):
    with open(fileName,'wb') as f:
        tensor.numpy().tofile(f)

# to add the support for padding & bias, see https://pytorch.org/docs/stable/generated/torch.nn.Conv2d.html
batchSize = 4
numInputChannels = 3
numOutputChannels = 3
stride = 1
kernelSize = 3
inputHeight = 160
inputWidth = 320

layer0 = torch.nn.Conv2d(numInputChannels, numInputChannels, 3, stride, 1, groups=numInputChannels, bias=True)
input = torch.randn(batchSize, numInputChannels, inputHeight, inputWidth, dtype=torch.float)
output = layer0(input)
output = layer0(input)
print(input.size())
print(layer0.weight.size())
print(output.size())
print(layer0.bias.size())
saveTensor2Bin(input, 'dw_conv_input3x3.bin')
saveTensor2Bin(layer0.weight.detach(), 'dw_conv_weight3x3.bin')
saveTensor2Bin(output.detach(), 'dw_conv_output_torch3x3.bin')
saveTensor2Bin(layer0.bias.detach(), 'dw_conv_bias3x3.bin') 