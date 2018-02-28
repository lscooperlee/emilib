#!/usr/bin/env python

import emilib
import sys

try:
    outdata = sys.argv[1].encode()
except IndexError:
    outdata = b''

m = emilib.emi_msg(msgnum = 1,
                   data = outdata,
                   flag = emilib.emi_flag.EMI_MSG_MODE_BLOCK)

ret = emilib.emi_msg_send(m)

print(ret)
