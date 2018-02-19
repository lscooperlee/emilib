#!/usr/bin/env python

import os
import sys
import time
import subprocess
import signal
from multiprocessing import Process, Value

import unittest
from emilib import *


class TestEmiLib(unittest.TestCase):
    def __init__(self, *args, **kwargs):
        super(TestEmiLib, self).__init__(*args, **kwargs)
        self.emi_core = None

    def setUp(self):
        self.emi_core = subprocess.Popen("emi_core", stdout = subprocess.PIPE, bufsize = 0)
        time.sleep(.5)

    def tearDown(self):
        self.emi_core.terminate()
        time.sleep(.5)

    def test_emi_addr(self):
        addr = emi_addr("127.1.1.2", 256)
        self.assertEqual(str(addr.ipv4), "127.1.1.2:256")

    def test_emi_msg(self):
        msg_data = emi_msg(
            data=b'1234'*1024, cmd=1, msgnum=2, flag=3, ipaddr="10.0.0.3")
        self.assertEqual(msg_data.data, b'1234'*1024)
        self.assertEqual(msg_data.msg, 2)
        self.assertEqual(msg_data.cmd, 1)
        self.assertEqual(msg_data.size, len(b'1234'*1024))
        self.assertEqual(str(msg_data.addr.ipv4), "10.0.0.3:1361")
        self.assertEqual(msg_data.is_block(), False)

    def test_emi_init(self):
        self.assertEqual(emi_init(), 0)

    def test_emi_msg_register(self):
        ret = emi_init()
        self.assertEqual(ret, 0)

        def func(msg):
            pass

        ret = emi_msg_register(1, func)
        self.assertEqual(ret, 0)

        self.assertRaises(EMIError, emi_msg_register, 1, func)

    def test_emi_msg_send_unblock_nosenddata(self):
        received = Value('i', 0)

        def recvprocess_unblock():
            emi_init()

            def func(msg):
                nonlocal received
                self.assertEqual(msg.msg, 2)
                self.assertEqual(msg.cmd, 1)

                with received.get_lock():
                    received.value += 1


            ret = emi_msg_register(2, func)
            self.assertEqual(ret, 0)

            signal.pause()
            time.sleep(2);

        p1 = Process(target = recvprocess_unblock)
        p2 = Process(target = recvprocess_unblock)

        p1.start()
        p2.start()

        time.sleep(1)

        msg = emi_msg(msgnum=2, cmd=1, ipaddr="127.0.0.1")
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        p1.join()
        p2.join()

        self.assertEqual(received.value, 2)

    def test_emi_msg_send_unblock_senddata(self):
        received= Value('i', 0)

        def recvprocess_unblock_data():
            emi_init()

            def func(msg):
                nonlocal received
                self.assertEqual(msg.msg, 3)
                self.assertEqual(msg.cmd, 1)
                self.assertEqual(msg.data, b"11112222"*1024)

                with received.get_lock():
                    received.value += 1


            ret = emi_msg_register(3, func)
            self.assertEqual(ret, 0)

            signal.pause()
            time.sleep(2);

        p1 = Process(target = recvprocess_unblock_data)
        p2 = Process(target = recvprocess_unblock_data)

        p1.start()
        p2.start()

        time.sleep(1)

        msg = emi_msg(msgnum=3, cmd=1, ipaddr="127.0.0.1", data=b'11112222'*1024)
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        p1.join()
        p2.join()

        self.assertEqual(received.value, 2)

    def test_emi_msg_send_block_noretdata(self):
        received= Value('i', 0)

        def recvprocess_block():
            emi_init()

            def func(msg):
                nonlocal received
                self.assertEqual(msg.msg, 4)
                self.assertEqual(msg.cmd, 1)
                self.assertEqual(msg.data, b"11112222"*1024)

                with received.get_lock():
                    received.value += 1

                return 0

            ret = emi_msg_register(4, func)
            self.assertEqual(ret, 0)

            signal.pause()
            time.sleep(2);

        p1 = Process(target = recvprocess_block)
        p2 = Process(target = recvprocess_block)

        p1.start()
        p2.start()

        time.sleep(1)

        msg = emi_msg(
            msgnum=4,
            cmd=1,
            ipaddr="127.0.0.1",
            data=b'11112222'*1024,
            flag=emi_flag.EMI_MSG_MODE_BLOCK)
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)

        p1.join()
        p2.join()

        self.assertEqual(received.value, 2)

        def recvprocess_block_fail():
            emi_init()

            def func(msg):
                nonlocal received
                self.assertEqual(msg.msg, 4)
                self.assertEqual(msg.cmd, 1)
                self.assertEqual(msg.data, b"11112222"*1024)

                with received.get_lock():
                    received.value += 1

                return -1

            ret = emi_msg_register(4, func)
            self.assertEqual(ret, 0)

            signal.pause()
            time.sleep(2);

        p1 = Process(target = recvprocess_block)
        p2 = Process(target = recvprocess_block)
        p3 = Process(target = recvprocess_block_fail)

        p1.start()
        p2.start()
        p3.start()

        time.sleep(1)

        msg = emi_msg(
            msgnum=4,
            cmd=1,
            ipaddr="127.0.0.1",
            data=b'11112222'*1024,
            flag=emi_flag.EMI_MSG_MODE_BLOCK)
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], -1)

        p1.join()
        p2.join()
        p3.join()

        self.assertEqual(received.value, 5)

    def test_emi_msg_send_block_retdata(self):
        received= Value('i', 0)

        def recvprocess_block_data():
            emi_init()

            def func(msg):
                nonlocal received
                self.assertEqual(msg.msg, 5)
                self.assertEqual(msg.cmd, 1)
                self.assertEqual(msg.data, b"11112222"*512)

                with received.get_lock():
                    received.value += 1

                return emi_load_retdata(msg, b'abcdef'*1024)

            ret = emi_msg_register(5, func)
            self.assertEqual(ret, 0)

            signal.pause()
            time.sleep(2);

        p1 = Process(target = recvprocess_block_data)
        p2 = Process(target = recvprocess_block_data)

        p1.start()
        p2.start()

        time.sleep(1)

        msg = emi_msg(
            msgnum=5,
            cmd=1,
            ipaddr="127.0.0.1",
            data=b'11112222'*512,
            flag=emi_flag.EMI_MSG_MODE_BLOCK)

        ret = emi_msg_send(msg)

        self.assertEqual(ret[0], -1)
        self.assertEqual(ret[1], b'abcdef'*1024)

        p1.join()
        p2.join()

        self.assertEqual(received.value, 2)

    def test_emi_msg_send_highlevel(self):
        emi_init()

        received = 0

        def func(msg):
            nonlocal received
            self.assertEqual(msg.msg, 6)
            self.assertEqual(msg.cmd, 1)
            self.assertEqual(msg.data, b"11112222")
            received = received + 1
            return emi_load_retdata(msg, 'abcdefghijkmln'.encode())

        ret = emi_msg_register(6, func)
        self.assertEqual(ret, 0)
        time.sleep(1)

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
            self.assertEqual(msg.msg, 7)
            self.assertEqual(msg.cmd, 1)
            self.assertEqual(msg.data, b"11112222")
            received = received + 1
            return emi_load_retdata(msg, 'abcdefghijkmln'.encode())

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
            self.assertEqual(msg.msg, 8)
            self.assertEqual(msg.cmd, 1)

        def func2(msg):
            self.assertEqual(func2.__name__, "func2")
            self.assertEqual(msg.msg, 9)
            self.assertEqual(msg.cmd, 1)

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

    def test_emi_msg_send_inside_msg_handler(self):
        emi_init()

        def func1(msg):
            self.assertEqual(func1.__name__, "func1")
            self.assertEqual(msg.msg, 10)
            self.assertEqual(msg.cmd, 1)

            msg = emi_msg(msgnum=11, cmd=1, ipaddr="127.0.0.1",
                        flag=emi_flag.EMI_MSG_MODE_BLOCK)
            ret = emi_msg_send(msg)

            self.assertEqual(ret[0], 0)
            self.assertEqual(ret[1], b'1234abcd')


        def func2(msg):
            self.assertEqual(func2.__name__, "func2")
            self.assertEqual(msg.msg, 11)
            self.assertEqual(msg.cmd, 1)
            return emi_load_retdata(msg, '1234abcd'.encode())

        ret = emi_msg_register(10, func1)
        self.assertEqual(ret, 0)

        ret = emi_msg_register(11, func2)
        self.assertEqual(ret, 0)

        msg = emi_msg(msgnum=10, cmd=1, ipaddr="127.0.0.1",
                    flag=emi_flag.EMI_MSG_MODE_BLOCK)
        ret = emi_msg_send(msg)
        self.assertEqual(ret[0], 0)


        time.sleep(1)


if __name__ == "__main__":
    unittest.main()
