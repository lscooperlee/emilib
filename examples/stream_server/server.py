
import time
import os

from http.server import BaseHTTPRequestHandler,HTTPServer
from socketserver import ThreadingMixIn
from io import StringIO
from PIL import Image

import emilib

img = b''
emilib.emi_init()

def func(msg):
    global img
    img = msg.data
    return 0

emilib.emi_msg_register(1, func)

class CamHandler(BaseHTTPRequestHandler):

    def do_GET(self):
        if self.path.endswith('.mjpg'):
            self.send_response(200)
            self.send_header('Content-type',
                             'multipart/x-mixed-replace; boundary=--jpgboundary')
            self.end_headers()
            while True:
                try:
                    self.wfile.write(b"--jpgboundary")
                    self.send_header('Content-type','image/jpeg')
                    self.send_header('Content-length', len(img))
                    self.end_headers()
                    self.wfile.write(img)
                    time.sleep(0.1)

                except KeyboardInterrupt:
                    break

        if self.path.endswith('index.html'):
            self.send_response(200)
            self.send_header('Content-type','text/html')
            self.end_headers()
            self.wfile.write(b'<html><head></head><body>')
            self.wfile.write(b'<img src="/cam.mjpg"/>')
            self.wfile.write(b'</body></html>')


class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    pass

def main():

    try:
        server = ThreadedHTTPServer(('0.0.0.0', 8080), CamHandler)
        server.serve_forever()
    except KeyboardInterrupt:
        server.socket.close()

if __name__ == '__main__':
    main()

