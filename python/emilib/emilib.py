import ctypes
import struct
import signal
import enum

_emilib = ctypes.cdll.LoadLibrary("libemi.so")

class emi_flag(enum.IntEnum):
    EMI_MSG_MODE_BLOCK = 0x00000100
    EMI_MSG_RET_SUCCEEDED = 0x00010000
    EMI_MSG_FLAG_ALLOCDATA = 0x00040000
    EMI_MSG_FLAG_EXCLUSIVE = 0x00008000
    EMI_MSG_FLAG_RETDATA = 0x00020000


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
        addr = ".".join([str(x)
                         for x in struct.unpack(">bbbb", self.sin_addr)])
        port = struct.unpack("H", (struct.pack(">H", self.sin_port)))[0]
        return "{0}:{1}".format(addr, port)


class emi_addr(ctypes.Structure):

    _fields_ = [
        ("ipv4", sockaddr_in),
        ("pid_t", ctypes.c_int),
        ("id", ctypes.c_uint),
    ]

    def __init__(self, ipaddr="0.0.0.0", port=1361):
        cipaddr = ctypes.c_char_p(ipaddr.encode())
        cport = ctypes.c_uint(port)
        _emilib.emi_fill_addr(ctypes.byref(self), cipaddr, cport)

    def __str__(self):
        return "emi_addr:{{ addr: {0}, pid: {1} }}".format(
            str(self.ipv4), str(self.pid_t), str(self.id))

#_emilib.get_offset.argtypes = (ctypes.c_void_p, ctypes.c_void_p)
#_emilib.get_offset.restype = ctypes.c_long

class emi_msg(ctypes.Structure):

    _fields_ = [
        ("addr", emi_addr),
        ("_flag", ctypes.c_uint, 32),
        ("msg", ctypes.c_uint, 32),
        ("cmd", ctypes.c_uint, 32),
        ("size", ctypes.c_uint, 32),
        ("retsize", ctypes.c_uint, 32),
        ("data_offset", ctypes.c_ulonglong, 64),
        ("retdata_offset", ctypes.c_ulonglong, 64),

        ("_data", ctypes.c_void_p),
    ]

    def __str__(self):
        return "emi_addr:{{ msg: {0}, cmd: {1} }}".format(
            str(self.msg), str(self.cmd))

    def __init__(self,
                 msgnum=0,
                 cmd=0,
                 data=b'',
                 ipaddr="127.0.0.1",
                 port=1361,
                 flag=0):
        cipaddr = ctypes.c_char_p(ipaddr.encode())

        ccmd = ctypes.c_uint(cmd)
        cflag = ctypes.c_uint(flag)
        cmsgnum = ctypes.c_uint(msgnum)

        self.size = len(data)
        self._data = ctypes.cast(ctypes.create_string_buffer(self.size), ctypes.c_void_p)
        self.data_offset = self._data - ctypes.addressof(self) 

        cdata = (ctypes.c_ubyte * self.size).from_buffer(bytearray(data))

        _emilib.emi_fill_msg(
            ctypes.byref(self), cipaddr, cdata, ccmd, cmsgnum, cflag)

    @property
    def data(self):
        addr = ctypes.addressof(self) + self.data_offset
        # ctypes.c_char is nul-terminated, thus may get bytes less then self.size
        ccharp = ctypes.cast(addr, ctypes.POINTER(ctypes.c_byte * self.size))
        return bytearray(ccharp.contents)

    @property
    def retdata(self):
        addr = ctypes.addressof(self) + self.retdata_offset
        ccharp = ctypes.cast(addr, ctypes.POINTER(ctypes.c_byte * self.retsize))
        return bytearray(ccharp.contents)

    @property
    def flag(self):
        flags = [ name for name, _ in emi_flag.__members__.items() if _ & self._flag ]
        return ",".join(flags)


_emilib.emi_init.argtypes = (None)
_emilib.emi_init.restype = ctypes.c_int

_emilib.emi_msg_register.argtypes = (
    ctypes.c_uint, ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(emi_msg)))
_emilib.emi_msg_register.restype = ctypes.c_int

_emilib.emi_msg_free_data.argtypes = (ctypes.POINTER(emi_msg),)

_emilib.emi_msg_send.argtypes = (ctypes.POINTER(emi_msg), )
_emilib.emi_msg_send.restype = ctypes.c_int

_emilib.emi_msg_send_highlevel.argtypes = (
    ctypes.c_char_p, ctypes.c_uint, ctypes.c_void_p, ctypes.c_size_t,
    ctypes.c_void_p, ctypes.c_size_t, ctypes.c_uint, ctypes.c_uint)
_emilib.emi_msg_send_highlevel.restype = ctypes.c_int

_emilib.emi_msg_prepare_return_data.argtypes = (ctypes.POINTER(emi_msg),
                                                ctypes.c_void_p, ctypes.c_uint)
_emilib.emi_msg_prepare_return_data.restype = ctypes.c_int


def emi_init():
    ret = _emilib.emi_init()
    if ret < 0:
        raise EMIError("emi_core did not run")
    return ret


_registered_callback = []


def emi_msg_register(msg_num, func):

    def callback_decorator(func):
        def f(msg):
            return func(msg.contents)
        return f

    num = ctypes.c_uint(msg_num)

    CMPFUNC = ctypes.CFUNCTYPE(ctypes.c_int, ctypes.POINTER(emi_msg))
    callback = CMPFUNC(callback_decorator(func))

    _registered_callback.append(callback)

    ret = _emilib.emi_msg_register(num, callback)
    return ret


def emi_msg_send(emi_msg):
    ret = _emilib.emi_msg_send(ctypes.pointer(emi_msg))
    retdata = bytes(emi_msg.retdata)
    _emilib.emi_msg_free_data(emi_msg)
    return ret, retdata


def emi_msg_send_highlevel(msgnum,
                           ipaddr="127.0.0.1",
                           cmd=0,
                           data=b'',
                           retsize=0,
                           block=False):
    cipaddr = ctypes.c_char_p(ipaddr.encode())
    cmsgnum = ctypes.c_uint(msgnum)
    ccmd = ctypes.c_uint(cmd)

    size = len(data)
    cssize = ctypes.c_size_t(size)
    csdata = (ctypes.c_ubyte * size).from_buffer(bytearray(data))

    crsize = ctypes.c_size_t(retsize)
    crdata = (ctypes.c_ubyte * retsize).from_buffer(bytearray(retsize))

    cflag = emi_flag.EMI_MSG_MODE_BLOCK if block else 0

    ret = _emilib.emi_msg_send_highlevel(
        cipaddr, cmsgnum, ctypes.pointer(csdata), cssize,
        ctypes.pointer(crdata), crsize, ccmd, ctypes.c_uint(cflag))
    return ret, bytes(crdata)


def emi_msg_prepare_return_data(msg, retbytes):
    size = len(retbytes)
    rdata = (ctypes.c_ubyte * size).from_buffer(bytearray(retbytes))
    rsize = ctypes.c_uint(size)
    return _emilib.emi_msg_prepare_return_data(msg, ctypes.byref(rdata), rsize)


def emi_loop():
    while True:
        signal.pause()


recorded_func = {}


def emi_register(msg_num):
    def emi_func_register(func):
        recorded_func[msg_num] = func
        return func

    return emi_func_register


def emi_run(loop=True):
    emi_init()

    for num, func in recorded_func.items():
        ret = emi_msg_register(num, func)
        if ret < 0:
            raise EMIError('emi msg register error')

    if loop:
        emi_loop()
