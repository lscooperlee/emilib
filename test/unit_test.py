#!/usr/bin/env python

import sys

import unittest
from emi_test import EmiTestor

BaseDir = EmiTestor.getBaseDir()
sys.path.append("{}/python/".format(BaseDir))
from emilib import emilib


class TestEmiLib(unittest.TestCase):

    def setUp(self):
        emicore=EmiTestor()
        emicore.runEmiCore()
        
    def test_emi_init(self):
        self.assertEqual(emilib.emi_init(), 0)



if __name__ == "__main__":
    unittest.main()