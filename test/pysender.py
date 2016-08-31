#!/usr/bin/env python

import emilib

m = emilib.emi_msg(msgnum = 1)
ret = emilib.emi_msg_send(m)
print(ret)

m = emilib.emi_msg(msgnum = 1, data = b'1234')
ret = emilib.emi_msg_send(m)
print(ret)

m = emilib.emi_msg(msgnum = 1, flag = emilib.emi_flag.EMI_MSG_MODE_BLOCK)
ret = emilib.emi_msg_send(m)
print(ret)

m = emilib.emi_msg(msgnum = 1, flag = emilib.emi_flag.EMI_MSG_MODE_BLOCK, data = b'4321')
ret = emilib.emi_msg_send(m)
print(ret)
