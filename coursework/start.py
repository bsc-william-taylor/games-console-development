#!/usr/bin/python
import sys
import os

if len(sys.argv) >= 2:
    os.chdir(os.path.dirname(os.path.abspath(sys.argv[0])))
    os.system(sys.argv[1])
else:
    print("didnt provide program name")