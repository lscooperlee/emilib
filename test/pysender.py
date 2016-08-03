#!/usr/bin/env python


import signal
import sys
sys.path.append("../python")

from emilib import *

if len(sys.argv) == 1:
    emilib.emi_msg_send_highlevel_nonblock("127.0.0.1",1,bytearray(b'hell'),2)
else:
    emilib.emi_msg_send_highlevel_nonblock(sys.argv[1],1,bytearray(b'hell'),2)
