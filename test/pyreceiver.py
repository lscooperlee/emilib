#!/usr/bin/env python


import signal
import sys
sys.path.append("../python")

from emilib import *



def func(emi):
    k=emilib.emi_convert_msg(emi)
    print(k)
    b=bytearray(b'world')
    ret=emilib.emi_msg_prepare_return_data(emi,b)
    print(ret)
    return 0



#emilib.emilib()
b=emilib.emi_init()
print(b)
b=emilib.emi_msg_register(1,func)
print(b)
emilib.emi_loop()
