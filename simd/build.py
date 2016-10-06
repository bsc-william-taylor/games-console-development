#!/usr/bin/python
import sys
import os

# Double check python version
print('Python version: ' + sys.version)
# Output so we know the script is running
print('Building...')
# Call C++
os.system('ppu-g++ -o app -O3 -maltivec *.cpp')
# Finished message
print('Finished')
