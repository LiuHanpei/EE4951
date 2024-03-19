import torch
import struct

def saveTensor2Bin(tensor, fileName):
    with open(fileName,'wb') as f:
        tensor.numpy().tofile(f)

# to add the support for padding & bias, see https://pytorch.org/docs/stable/generated/torch.nn.Conv2d.html
batchSize = 4
numInputChannels = 3
numOutputChannels = 48
stride = 1
kernelSize = 3
inputHeight = 160
inputWidth = 320

layer0 = torch.nn.Conv2d(numInputChannels, numOutputChannels, 1, 1, 0, bias=True)
input = torch.randn(batchSize, numInputChannels, inputHeight, inputWidth, dtype=torch.float)
output = layer0(input)
output = layer0(input)
# print(input.size())
# print(layer0.weight.size())
# print(output.size())
# print(layer0.bias.size())

saveTensor2Bin(input, 'conv_input1x1.bin')
saveTensor2Bin(layer0.weight.detach(), 'conv_weight1x1.bin')
saveTensor2Bin(output.detach(), 'conv_output_torch1x1.bin')
saveTensor2Bin(layer0.bias.detach(), 'conv_bias1x1.bin') 
