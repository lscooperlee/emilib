#!/usr/env/python

import os
from itertools import product
import subprocess
import time

Rules = [
    {"description" : "unblock sender, single receiver, without sent data",
     "sender": "-s 127.0.0.1 -m 1 -c 2", "receiver": "-r 1"},

    {"description" : "unblock sender, single receiver, with sent data",
     "sender": "-s 127.0.0.1 -m 1 -c 2 -d 0123456789abcdefABCDEF", "receiver": "-r 1"},

    {"description" : "block sender, single receiver, without sent data, without ret data",
     "sender": "-b -s 127.0.0.1 -m 1 -c 2", "receiver": "-b -r 1"},

    {"description" : "block sender, single receiver, with sent data, without ret data",
     "sender": "-b -s 127.0.0.1 -m 1 -c 2 -d 0123456789abcdefABCDEF", "receiver": "-b -r 1"},

    {"description" : "block sender, single receiver, without sent data, with ret data",
     "sender": "-b -s 127.0.0.1 -m 1 -c 2 -D 256", "receiver": "-b -r 1 -R abcdefABCDEF0123456789"},

    {"description" : "block sender, single receiver, with sent data, with ret data",
     "sender": "-b -s 127.0.0.1 -m 1 -c 2 -d 0123456789abcdefABCDEF -D 256", "receiver": "-b -r 1 -R abcdefABCDEF0123456789"},
]

Cmds = [
    {"cmd" : "emi_sar"}
]

class EmiTestor:

    BASEDIR = os.path.abspath(
        os.path.join(os.path.dirname(os.path.realpath(__file__)), os.path.pardir))
    BINDIR = os.path.join(BASEDIR, ".out/bin")
    LIBDIR = os.path.join(BASEDIR, ".out/lib")
    
    def runEmiCore(self):
        return self.runCMD("emi_core")

    def run(self):
        emiCore = self.runEmiCore()

        try:
            cmds = product(Cmds, Rules)
            for _cmd in cmds:
                description = "{[1][description]}".format(_cmd)
                senderWithArgs = "{0[0][cmd]} {0[1][sender]}".format(_cmd)
                receiverWithArgs = "{0[0][cmd]} {0[1][receiver]}".format(_cmd)

                print(description)
                time.sleep(1)      
                receiver = self.runCMD(receiverWithArgs)
                time.sleep(1)      
                sender = self.runCMD(senderWithArgs)
                time.sleep(1)      

                receiver.terminate()
                sender.terminate()

        finally:
            emiCore.terminate()
        
    def runCMD(self, cmd):
        fullPathCmd = "{0}/{1}".format(self.BINDIR, cmd)
        return subprocess.Popen(fullPathCmd.split(), env={"LD_LIBRARY_PATH" : self.LIBDIR})


if __name__ == "__main__":
    testor = EmiTestor()
    testor.run()
