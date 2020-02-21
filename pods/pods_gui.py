#!/usr/bin/python3
import subprocess 
import io
from time import sleep
import cppyy
import os

os.system('touch display_launched')
it = 0
while it < 10:
    n = input()
    sleep(1)
    
    print('Python reads {}'.format(n))
    it += 1
