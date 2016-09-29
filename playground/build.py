#!/usr/bin/python
import sys
import os

length = len(sys.argv)

build_ppu = length >= 2 and sys.argv[1] == 'y'
build_spu = length >= 2 and sys.argv[2] == 'y'

if build_ppu:
    print('building PPU exe...')
    os.system('ppu-g++ -o ppu/ppu ppu/*.cpp -lspe2')
if build_spu:
    print('building SPU exe...')
    os.system('spu-g++ -o spu/spu spu/*.cpp')
