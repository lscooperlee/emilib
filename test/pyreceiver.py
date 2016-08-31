#!/usr/bin/env python

import emilib


@emilib.emi_register(1)
def func(emi):
    msgnum = emi.contents.msg
    cmd = emi.contents.cmd
    data = emi.contents.data
    flag = emi.contents.flag
    print((msgnum, cmd, data))
    
    if flag & emilib.emi_flag.EMI_MSG_MODE_BLOCK and data:
        retdata = b'1' * emi.contents.size

        emilib.emi_msg_prepare_return_data(emi, retdata)

    return 0



emilib.emi_run(True)
