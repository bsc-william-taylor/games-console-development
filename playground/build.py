#!/usr/bin/python
import sys
import os

# Double check python version
print('python version: ' + sys.version)
# Output so we know the script is running
print('building PPU exe...')
os.system('ppu-g++ -o ppu/ppu ppu/*.cpp -lspe2')
print('finished')
print('building spu exe...')
os.system('spu-g++ -o spu/spu spu/*.cpp')
print('finished')
