#!/usr/bin/python
import sys
import os

# Double check python version
print('python version: ' + sys.version)
# Output so we know the script is running
print('building PPU exe...')
os.system('g++ -o ppu/ppu -std=c++0x ppu/*.cpp')
print('finished')
print('building spu exe...')
os.system('spu-g++ -o spu/spu spu/*.cpp')
print('finished')
