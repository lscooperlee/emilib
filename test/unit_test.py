#!/usr/bin/env python

import sys

import unittest
from emi_test import EmiTestor

BaseDir = EmiTestor.getBaseDir()
sys.path.append("{}/python/".format(BaseDir))
from emilib import emilib


class TestEmiLib(unittest.TestCase):

    def __init__(self, *args, **kwargs):
        super(TestEmiLib, self).__init__(*args, **kwargs)
        self.emiTestor = EmiTestor()

    def setUp(self):
        self.emiTestor.runEmiCore()

    def tearDown(self):
        self.emiTestor.stopEmiCore()
        
    def test_emi_init(self):
        self.assertEqual(emilib.emi_init(), 0)
        
    def test_emi_msg_register(self):
        def func():
            print("emi registered")
        ret = emilib.emi_msg_register(1,func)
        self.assertEqual(ret, 0)



if __name__ == "__main__":
    unittest.main()