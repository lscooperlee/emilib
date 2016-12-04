#!/usr/bin/env python

import emilib


#@emilib.emi_register(1)
def func(emi):
    msgnum = emi.msg
    cmd = emi.cmd
    data = emi.data
    flag = emi.flag
    print((msgnum, cmd, data))

    #if emi.is_block():
    #    retdata = b'1111'
    #    return retdata

    return b'1111'



#emilib.emi_run(True)

emilib.emi_init()

emilib.emi_msg_register(1, func)

emilib.emi_loop()
