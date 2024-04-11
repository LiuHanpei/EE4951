import torch
import struct

def saveTensor2Bin(tensor, fileName):
    with open(fileName,'wb') as f:
        tensor.numpy().tofile(f)


batchSize = 1
numInputChannels = 3
numOutputChannels = 48
strideSize = 1
kernelSize = 3
inputHeight = 160
inputWidth = 320
paddingSize = 1

layer0 = torch.nn.Conv2d(numInputChannels, numInputChannels, kernel_size=kernelSize, stride=strideSize, padding=paddingSize, groups = numInputChannels, bias=True)
input0 = torch.randn(batchSize, numInputChannels, inputHeight, inputWidth, dtype=torch.float)
output0 = layer0(input0)

layer1 = torch.nn.Conv2d(numInputChannels, numOutputChannels, kernel_size=kernelSize, stride=strideSize, padding=paddingSize, groups = numInputChannels, bias=True)
input1 = output0
output1 = layer1(input1)
# output = layer0(input)
print('input0 size:',input0.size())
print('weight0 size:',layer0.weight.size())
print('bias0 size:',layer0.bias.size())
print('output0 size:',output0.size())
print('input1 size:',input1.size())
print('weight1 size:',layer1.weight.size())
print('bias1 size:',layer1.bias.size())
print('output1 size:',output1.size())

saveTensor2Bin(input0, 'conv_input.bin')
saveTensor2Bin(layer0.weight.detach(), 'conv_weight0.bin')
saveTensor2Bin(layer0.bias.detach(), 'conv_bias.bin0')
saveTensor2Bin(layer1.weight.detach(), 'conv_weight1.bin')
saveTensor2Bin(layer1.bias.detach(), 'conv_bias.bin1')
saveTensor2Bin(output1.detach(), 'conv_output.bin')
# print(output)