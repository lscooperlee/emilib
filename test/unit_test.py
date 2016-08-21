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

    def tearDown(self):
        self.emiTestor.stopEmiCore()

    def test_emi_fill_addr(self):
        addr = emi_addr()
        print(addr)
        emilib.emi_fill_addr(addr, "127.1.1.2", 256)
        print(addr)

    def test_emi_msg(self):
        msg = emi_msg()
        print(dir(msg))
        print(msg.data)

    def test_emi_init(self):
         self.assertEqual(emilib.emi_init(), 0)

    def test_emi_msg_register(self):
        emilib.emi_init()
        def func():
            print("emi registered")
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

        def func(emi_msg):
            print("python: emi unblock nosenddata")
            print(emi_msg)
            return 0

        ret = emilib.emi_msg_register(2, func)
        self.assertEqual(ret, 0)
 
        ret = emilib.emi_msg_send_highlevel("127.0.0.1", 2, 1)
         
        time.sleep(1)

    def test_emi_msg_send_highlevel_unblock_senddata(self):
        emilib.emi_init()
        def func(emi_msg):
            print("python: emi registered")
            print(emi_msg.contents.data)
            return 0
        ret = emilib.emi_msg_register(3, func)
        self.assertEqual(ret, 0)

        ret = emilib.emi_msg_send_highlevel("127.0.0.1", 3, 1, b"11112222")
        
        time.sleep(1)
    

if __name__ == "__main__":
    unittest.main()
