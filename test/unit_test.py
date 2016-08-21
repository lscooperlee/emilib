#!/usr/bin/env python

import os
import sys
import time

import unittest
from emi_test import EmiTestor
from emilib import emilib, emi_msg, emi_addr, sockaddr_in

class TestEmiLib(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestEmiLib, self).__init__(*args, **kwargs)
        self.emiTestor = EmiTestor()

    def setUp(self):
        self.emiTestor.runEmiCore()
        time.sleep(0.2)

    def tearDown(self):
        self.emiTestor.stopEmiCore()

    def test_emi_addr(self):
        addr = emi_addr("127.1.1.2", 256)
        self.assertEqual(str(addr.ipv4), "127.1.1.2:256")

    def test_emi_msg(self):
        msg = emi_msg()
        msg_data = emi_msg(data=b'1234', cmd=1, msgnum=2, flag=3, ipaddr="10.0.0.3")
        self.assertEqual(msg_data.data, b'1234')
        self.assertEqual(msg_data.msg, 2)
        self.assertEqual(msg_data.cmd, 1)
        self.assertEqual(msg_data.size, len(b'1234'))
        self.assertEqual(str(msg_data.dest_addr.ipv4), "10.0.0.3:1361")

    def test_emi_init(self):
         self.assertEqual(emilib.emi_init(), 0)

    def test_emi_msg_register(self):
        emilib.emi_init()
        def func(msg):
            return 0
        ret = emilib.emi_msg_register(1, func)
        self.assertEqual(ret, 0)

#      def test_emi_msg_send(self):
#          def func():
#              print("emi registered")
#          ret = emilib.emi_msg_register(2, func)
#          self.assertEqual(ret, 0)
#          
#          msg = emi_msg()
#          msg.msg = 2
#          ret = emilib.emi_msg_send(msg)
#          print(msg)
#          print(ret)

    def test_emi_msg_send_highlevel_unblock_nosenddata(self):
        emilib.emi_init()
        
        received = True
        def func(msg):
            nonlocal received
            self.assertEqual(received, True)
            self.assertEqual(msg.contents.msg, 2)
            self.assertEqual(msg.contents.cmd, 1)
            received = False
            return 0

        ret = emilib.emi_msg_register(2, func)
        self.assertEqual(ret, 0)
 
        ret = emilib.emi_msg_send_highlevel("127.0.0.1", 2, 1)
         
        time.sleep(1)

        self.assertEqual(received, False)

    def test_emi_msg_send_highlevel_unblock_senddata(self):
        emilib.emi_init()

        received = True
        def func(msg):
            nonlocal received
            self.assertEqual(received, True)
            self.assertEqual(msg.contents.msg, 3)
            self.assertEqual(msg.contents.cmd, 1)
            self.assertEqual(msg.contents.data, b"11112222")
            received = False
            return 0

        ret = emilib.emi_msg_register(3, func)
        self.assertEqual(ret, 0)

        ret = emilib.emi_msg_send_highlevel("127.0.0.1", 3, 1, b"11112222")
        
        time.sleep(1)

        self.assertEqual(received, False)
    

if __name__ == "__main__":
    unittest.main()
