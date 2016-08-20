#!/usr/bin/env python

import ctypes
import struct

_emilib = ctypes.cdll.LoadLibrary("libemi.so")

class EMIError(Exception):

    def __init__(cls, name):
        cls.__name = name

    def __str__(cls):
        return cls.__name

class sockaddr_in(ctypes.Structure):
    _fields_ = [
                ("sa_family", ctypes.c_ushort, 16),  # sin_family
                ("sin_port", ctypes.c_ushort, 16),
                ("sin_addr", ctypes.c_byte * 4),
                ("__pad", ctypes.c_char * 8)
            ] 

#    def __init__(self, ipaddr="127.0.0.1", port = 1361):
#        pass
#        cipaddr = ctypes.c_char_p(ipaddr.encode())
#        cport = ctypes.c_ushort(port)
#        super(sockaddr_in, self).__init__(0, cport, cipaddr, 0)

    def __str__(self):
        addr = ",".join([ str(x) for x in struct.unpack(">bbbb", self.sin_addr)])
        port = struct.unpack("H", (struct.pack(">H", self.sin_port)))[0]
        return "{0}:{1}".format(addr, port)

class emi_addr(ctypes.Structure):

    _fields_ = [
                ("ipv4", sockaddr_in),
                ("pid_t", ctypes.c_int),
                ("id", ctypes.c_uint),
            ]

    def __init__(self, ipaddr="0.0.0.0", port = 1361):
        cipaddr = ctypes.c_char_p(ipaddr.encode())
        cport = ctypes.c_uint(port)
        _emilib.emi_fill_addr(ctypes.pointer(self), cipaddr, cport)

    def __str__(self):
        return "emi_addr:{{ addr: {0}, pid: {1} }}".format(str(self.ipv4), str(self.pid_t))

class emi_msg(ctypes.Structure):

    _fields_=[
                ("dest_addr", emi_addr),
                ("src_addr", emi_addr),
                ("flag", ctypes.c_uint, 32),
                ("count", ctypes.c_uint, 32),
                ("size", ctypes.c_uint, 32),
                ("cmd", ctypes.c_uint, 32),
                ("msg", ctypes.c_uint, 32),
            ]

    def __str__(self):
        return "emi_addr:{{ msg: {0}, cmd: {1} }}".format(str(self.msg), str(self.cmd))

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.data = ctypes.c_char_p(ctypes.addressof(self)+ctypes.sizeof(self))


class emilib:

    __emilib = _emilib

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

    __emilib.emi_fill_addr.argtypes = (ctypes.POINTER(emi_addr), ctypes.c_char_p, ctypes.c_uint)
    __emilib.emi_fill_addr.restype = ctypes.c_int


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
    def emi_fill_addr(cls, emiaddr, ipaddr, port):
        cipaddr = ctypes.c_char_p(ipaddr.encode())
        cport = ctypes.c_uint(port)
        return cls.__emilib.emi_fill_addr(ctypes.pointer(emiaddr), cipaddr, cport)

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

