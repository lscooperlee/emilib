#!/usr/bin/env python

from ctypes import *
import signal


class EMIError(Exception):
    def __init__(self, name):
        self.__name = name

    def __str__(self):
        return self.__name


class emi(Structure):
    _fields_ = []
#    _fields_=[("msg",c_int),("cmd",c_int)]


class emilib:

    __emilib = cdll.LoadLibrary("libemi.so")

    def __init__(self):
        pass

    @classmethod
    def emi_init(self):
        ret = c_int(0)
        ret = self.__emilib.emi_init()
        if ret < 0:
            raise EMIError("emi_core did not run")
        return ret

    @classmethod
    def emi_msg_register(self, msg_num, func, block=False):
        ret = c_int(-1)
        num = c_int(msg_num)
        CMPFUNC = CFUNCTYPE(c_int, POINTER(emi))
        callback = CMPFUNC(func)
        if block:
            ret = self.__emilib.emi_msg_register_block(num, callback)
        else:
            ret = self.__emilib.emi_msg_register(num, callback)
        return ret

    @classmethod
    def emi_msg_send_highlevel_block(self, ipaddr, msgnum, sbytes, cmd):
        cip = c_char_p(ipaddr)
        print(cip)
#    def emi_msg_send_highlevel_block(char *ipaddr, int msgnum,void *send_data,int send_size,void *ret_data,  int ret_size,eu32 cmd);

    @classmethod
    def emi_msg_send_highlevel_nonblock(self, ipaddr, msgnum, sbytes, cmd):
        cip = c_char_p(ipaddr.encode())
        cnum = c_int(msgnum)
        ccmd = c_int(cmd)

        size = len(sbytes)
        csize = c_int(size)
        cdata = (c_ubyte * size).from_buffer(sbytes)

        return self.__emilib.emi_msg_send_highlevel_nonblock(
            cip, cnum, pointer(cdata), csize, ccmd)

    @classmethod
    def emi_convert_msg(self, msg):
        ret = {}
        msgnum = c_int(-1)
        cmd = c_int(-1)
        size = c_int(-1)

        msgnum = self.__emilib.get_msgnum_from_msg(msg)
        cmd = self.__emilib.get_cmd_from_msg(msg)
        size = self.__emilib.get_datasize_from_msg(msg)

        cdata = (c_ubyte * size)()
        self.__emilib.copy_data_from_msg(msg, cdata)

        ret["msg"] = msgnum
        ret["cmd"] = cmd
        ret["size"] = size
        ret["data"] = bytes(cdata)

        return ret

    @classmethod
    def emi_msg_prepare_return_data(self, msg, retbytes):
        size = len(retbytes)
        rdata = (c_ubyte * size).from_buffer(retbytes)
        rsize = c_int(size)
        return self.__emilib.emi_msg_prepare_return_data(msg, byref(rdata),
                                                         rsize)

    @classmethod
    def emi_loop(self):
        while True:
            signal.pause()