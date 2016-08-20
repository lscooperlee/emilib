#!/usr/bin/env python

import ctypes


class EMIError(Exception):

    def __init__(cls, name):
        cls.__name = name

    def __str__(cls):
        return cls.__name

class sockaddr_in(ctypes.Structure):
    _fields_ = [
                ("sa_family", ctypes.c_ushort, 16),  # sin_family
                ("sin_port", ctypes.c_ushort, 16),
                ("sin_addr", ctypes.c_char * 4),
                ("__pad", ctypes.c_char * 8)
            ] 


class emi_addr(ctypes.Structure):
    _fields_ = [
                ("ipv4", sockaddr_in),
                ("pid_t", ctypes.c_int),
                ("id", ctypes.c_uint),
            ]

class emi_msg(ctypes.Structure):
    _fields_=[
                ("dest_addr", emi_addr),
                ("src_addr", emi_addr),
                ("flag", ctypes.c_uint, 32),
                ("count", ctypes.c_uint, 32),
                ("size", ctypes.c_uint, 32),
                ("cmd", ctypes.c_uint, 32),
                ("msg", ctypes.c_uint, 32),
                ("data", ctypes.c_void_p),
            ]

class emilib:

    __emilib = ctypes.cdll.LoadLibrary("libemi.so")
    __emilib.emi_init.argtypes = (None)
    __emilib.emi_init.restype = ctypes.c_int

    __emilib.emi_msg_register.argtypes = (ctypes.c_uint, 
            ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(emi_msg)))
    __emilib.emi_msg_register.restype = ctypes.c_int

    __emilib.emi_msg_send.argtypes = (ctypes.POINTER(emi_msg),)
    __emilib.emi_msg_send.restype = ctypes.c_int

    __emilib.emi_msg_send_highlevel.argtypes = (ctypes.c_char_p, ctypes.c_uint, 
            ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, 
            ctypes.c_size_t, ctypes.c_uint, ctypes.c_uint)
    __emilib.emi_msg_send_highlevel.restype = ctypes.c_int


    @classmethod
    def emi_init(cls):
        ret = ctypes.c_int(0)
        ret = cls.__emilib.emi_init()
        if ret < 0:
            raise EMIError("emi_core did not run")
        return ret

    @classmethod
    def emi_msg_register(cls, msg_num, func):
        ret = ctypes.c_int(0)
        num = ctypes.c_uint(msg_num)
        CMPFUNC = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(emi_msg))
        callback = CMPFUNC(func)
        ret = cls.__emilib.emi_msg_register(num, callback)
        return ret

    @classmethod
    def emi_msg_send(cls, emi_msg):
        ret = ctypes.c_int(0)
        ret = cls.__emilib.emi_msg_send(emi_msg)
        return ret

    @classmethod
    def emi_msg_send_highlevel(cls, ipaddr, msgnum, cmd, sbytes=b'', block=False):
        cipaddr = ctypes.c_char_p(ipaddr.encode())
        cmsgnum = ctypes.c_uint(msgnum)
        ccmd = ctypes.c_uint(cmd)

        size = len(sbytes)
        cssize = ctypes.c_size_t(size)
        csdata = (ctypes.c_ubyte * size).from_buffer(bytearray(sbytes))

#        crsize = ctypes.c_int(size)
#        crdata = (ctypes.c_ubyte * size).from_buffer(sbytes[:])
        crsize = ctypes.c_size_t(size)
        crdata = (ctypes.c_ubyte * size).from_buffer(bytearray(size))

        return cls.__emilib.emi_msg_send_highlevel(cipaddr, cmsgnum, 
                ctypes.pointer(csdata), cssize, ctypes.pointer(crdata), crsize,
                ccmd, ctypes.c_uint(0))

# 
#     @classmethod
#     def emi_msg_prepare_return_data(cls, msg, retbytes):
#         size = len(retbytes)
#         rdata = (c_ubyte * size).from_buffer(retbytes)
#         rsize = c_int(size)
#         return cls.__emilib.emi_msg_prepare_return_data(msg, byref(rdata),
#                                                          rsize)
    @classmethod
    def emi_loop(cls):
        cls.__emilib.emi_loop()

