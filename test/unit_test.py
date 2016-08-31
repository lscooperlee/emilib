#!/usr/bin/env python

import os
import sys
import time

import unittest
from emi_test import EmiTestor
from emilib import *


class TestEmiLib(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestEmiLib, self).__init__(*args, **kwargs)
        self.emiTestor = EmiTestor()

    def setUp(self):
        self.emiTestor.runEmiCore()
        time.sleep(.2)

    def tearDown(self):
        self.emiTestor.stopEmiCore()

    def test_emi_addr(self):
        addr = emi_addr("127.1.1.2", 256)
        self.assertEqual(str(addr.ipv4), "127.1.1.2:256")

    def test_emi_msg(self):
        msg_data = emi_msg(
            data=b'1234', cmd=1, msgnum=2, flag=3, ipaddr="10.0.0.3")
        self.assertEqual(msg_data.data, b'1234')
        self.assertEqual(msg_data.msg, 2)
        self.assertEqual(msg_data.cmd, 1)
        self.assertEqual(msg_data.size, len(b'1234'))
        self.assertEqual(str(msg_data.dest_addr.ipv4), "10.0.0.3:1361")

    def test_emi_msg_prepare_return_data(self):
        msg_data = emi_msg(
            data=b'87654321',
            cmd=2,
            msgnum=1,
            flag=emi_flag.EMI_MSG_MODE_BLOCK,
            ipaddr="127.0.0.3")
        self.assertEqual(msg_data.data, b'87654321')
        self.assertEqual(msg_data.flag, emi_flag.EMI_MSG_MODE_BLOCK)
        self.assertEqual(msg_data.size, len(b'87654321'))
        self.assertEqual(str(msg_data.dest_addr.ipv4), "127.0.0.3:1361")

        ret = emi_msg_prepare_return_data(msg_data, b'abcdef')

        self.assertEqual(ret, 0)
        self.assertEqual(msg_data.data, b'abcdef')
        self.assertEqual(msg_data.flag, emi_flag.EMI_MSG_MODE_BLOCK |
                         emi_flag.EMI_MSG_RET_WITHDATA)
        self.assertEqual(msg_data.size, len(b'abcdef'))
        self.assertEqual(str(msg_data.dest_addr.ipv4), "127.0.0.3:1361")

    def test_emi_init(self):
        self.assertEqual(emi_init(), 0)

    def test_emi_msg_register(self):
        ret = emi_init()
        self.assertEqual(ret, 0)

        def func(msg):
            return 0

        ret = emi_msg_register(1, func)

        self.assertEqual(ret, 0)

    def test_emi_msg_send_unblock_nosenddata(self):
        emi_init()

        received = 0

        def func(msg):
            nonlocal received
            self.assertEqual(msg.contents.msg, 2)
            self.assertEqual(msg.contents.cmd, 1)
            received = received + 1
            return 0

        ret = emi_msg_register(2, func)
        self.assertEqual(ret, 0)

        msg = emi_msg(msgnum=2, cmd=1, ipaddr="127.0.0.1")
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        time.sleep(1)

        self.assertEqual(received, 1)

    def test_emi_msg_send_unblock_senddata(self):
        emi_init()

        received = 0

        def func(msg):
            nonlocal received
            self.assertEqual(msg.contents.msg, 3)
            self.assertEqual(msg.contents.cmd, 1)
            self.assertEqual(msg.contents.data, b"11112222")
            received = received + 1
            return 0

        ret = emi_msg_register(3, func)
        self.assertEqual(ret, 0)

        msg = emi_msg(msgnum=3, cmd=1, ipaddr="127.0.0.1", data=b'11112222')
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        time.sleep(1)

        self.assertEqual(received, 1)

    def test_emi_msg_send_block_noretdata(self):
        emi_init()

        received = 0

        def func(msg):
            nonlocal received
            self.assertEqual(msg.contents.msg, 4)
            self.assertEqual(msg.contents.cmd, 1)
            self.assertEqual(msg.contents.data, b"11112222")
            received = received + 1
            return 0

        ret = emi_msg_register(4, func)
        self.assertEqual(ret, 0)

        msg = emi_msg(
            msgnum=4,
            cmd=1,
            ipaddr="127.0.0.1",
            data=b'11112222',
            flag=emi_flag.EMI_MSG_MODE_BLOCK)
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        time.sleep(1)

        self.assertEqual(received, 1)

    def test_emi_msg_send_block_retdata(self):
        emi_init()

        received = 0

        def func(msg):
            nonlocal received
            self.assertEqual(msg.contents.msg, 5)
            self.assertEqual(msg.contents.cmd, 1)
            self.assertEqual(msg.contents.data, b"11112222")
            received = received + 1
            emi_msg_prepare_return_data(msg, b'abcdefghijkmln')
            return 0

        ret = emi_msg_register(5, func)
        self.assertEqual(ret, 0)

        msg = emi_msg(
            msgnum=5,
            cmd=1,
            ipaddr="127.0.0.1",
            data=b'11112222',
            flag=emi_flag.EMI_MSG_MODE_BLOCK)
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)
        self.assertEqual(ret[1], b'abcdefghijkmln')

        time.sleep(1)

        self.assertEqual(received, 1)

    def test_emi_msg_send_highlevel(self):
        emi_init()

        received = 0

        def func(msg):
            nonlocal received
            self.assertEqual(msg.contents.msg, 6)
            self.assertEqual(msg.contents.cmd, 1)
            self.assertEqual(msg.contents.data, b"11112222")
            received = received + 1
            emi_msg_prepare_return_data(msg, b'abcdefghijkmln')
            return 0

        ret = emi_msg_register(6, func)
        self.assertEqual(ret, 0)

        ret = emi_msg_send_highlevel(
            msgnum=6,
            cmd=1,
            ipaddr="127.0.0.1",
            data=b'11112222',
            retsize=len(b'abcdefghijkmln'),
            block=True)
        self.assertEqual(ret[0], 0)
        self.assertEqual(ret[1], b'abcdefghijkmln')

        time.sleep(1)

        self.assertEqual(received, 1)

    def test_register_decorator(self):

        received = 0

        @emi_register(7)
        def func(msg):
            nonlocal received
            self.assertEqual(msg.contents.msg, 7)
            self.assertEqual(msg.contents.cmd, 1)
            self.assertEqual(msg.contents.data, b"11112222")
            received = received + 1
            emi_msg_prepare_return_data(msg, b'abcdefghijkmln')
            return 0

        emi_run(False)

        ret = emi_msg_send_highlevel(
            msgnum=7,
            cmd=1,
            ipaddr="127.0.0.1",
            data=b'11112222',
            retsize=len(b'abcdefghijkmln'),
            block=True)
        self.assertEqual(ret[0], 0)
        self.assertEqual(ret[1], b'abcdefghijkmln')

        time.sleep(1)

        self.assertEqual(received, 1)

    def test_emi_msg_send_multiple_func(self):
        emi_init()

        def func1(msg):
            self.assertEqual(func1.__name__, "func1")
            self.assertEqual(msg.contents.msg, 8)
            self.assertEqual(msg.contents.cmd, 1)
            return 0

        def func2(msg):
            self.assertEqual(func2.__name__, "func2")
            self.assertEqual(msg.contents.msg, 9)
            self.assertEqual(msg.contents.cmd, 1)
            return 0

        ret = emi_msg_register(8, func1)
        self.assertEqual(ret, 0)

        ret = emi_msg_register(9, func2)
        self.assertEqual(ret, 0)

        msg = emi_msg(msgnum=8, cmd=1, ipaddr="127.0.0.1")
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        msg = emi_msg(msgnum=9, cmd=1, ipaddr="127.0.0.1")
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        time.sleep(1)


if __name__ == "__main__":
    unittest.main()
