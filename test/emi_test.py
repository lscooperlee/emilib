#!/usr/bin/env python

import os, sys
from itertools import product
import subprocess
import time
import io

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
    
    def runEmiCore(self):
        self.emiCore = self.runCMD("emi_core")

    def stopEmiCore(self):
        self.emiCore.terminate()

    def run(self):
        emiCore = self.runEmiCore()

        try:
            cmds = product(Cmds, Rules)
            for _cmd in cmds:
                description = "{[1][description]}".format(_cmd)
                senderWithArgs = "{0[0][cmd]} {0[1][sender]}".format(_cmd)
                receiverWithArgs = "{0[0][cmd]} {0[1][receiver]}".format(_cmd)

                print(description)
                receiver = self.runCMD(receiverWithArgs)

                sender = self.runCMD(senderWithArgs)
                sender.wait()
                s = sender.communicate() 
                sender.terminate()

                r = None
                try:
                    receiver.communicate(timeout = 1)
                except subprocess.TimeoutExpired:
                    receiver.terminate()
                    r = receiver.communicate()
                
                if s != r:
                    print("sender output: {}".format(s))
                    print("receiver output: {}".format(r))
                    sys.exit(-1)                

        finally:
            self.stopEmiCore()
        
    def runCMD(self, cmd):
        return subprocess.Popen(cmd.split(), stdout = subprocess.PIPE, bufsize = 0)

if __name__ == "__main__":
    testor = EmiTestor()
    testor.run()
