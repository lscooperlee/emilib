import cv2
import emilib
import time

camera = cv2.VideoCapture(0)

while True:
    ret, im = camera.read()

    ret, jpg = cv2.imencode(".jpg", im)

    msg = emilib.emi_msg(
        msgnum=1,
        data=jpg.tobytes(),
        ipaddr="127.0.0.1", # Change to the IP where server.py is in
        flag=emilib.emi_flag.EMI_MSG_MODE_BLOCK
    )

    ret = emilib.emi_msg_send(msg)

del(camera)
