#!/usr/bin/env python

import emilib


@emilib.emi_register(1)
def func(msg):
    print((msg.msg, msg.cmd, msg.data))
    if msg.data:
        emilib.emi_load_retdata(msg, msg.data[::-1])
    return 0

emilib.emi_run()
