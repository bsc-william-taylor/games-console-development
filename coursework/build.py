#!/usr/bin/python
import platform
import sys
import os

working_directory = os.path.dirname(os.path.abspath(sys.argv[0]))
optimisations = False
build_spu = False
build_ppu = False

os_platform = platform.platform()
py_version = sys.version

print(working_directory)
print(os_platform)
print(py_version)

os.chdir(working_directory)

def build(output, input):
    cmd = 'spu-g++ -o ' + output + ' ' + input
    if optimisations == True:
        cmd += ' -O2'
    os.system(cmd)

if len(sys.argv) > 1:
    for arg in sys.argv:
        if arg == "-spu":
            build_spu = True
        if arg == "-ppu":
            build_ppu = True   
        if arg == "-r":
            optimisations = True 

if build_ppu:
    print('building ppu source')
    if "Window" not in os_platform:
        cmd = 'ppu-g++ -o ppu/ppu ../common/*.cpp ppu/*.cpp -lspe2 '
        if optimisations == True:
            cmd += ' -O2'
        os.system(cmd)

if build_spu:
    print('building spu source')
    if "Window" not in os_platform:
        build('blur/blur', 'blur/*.cpp')
        build('sobel/sobel', 'sobel/*.cpp')
        