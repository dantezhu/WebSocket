#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
pip install websocket-client
"""

import json
import time

from websocket import create_connection
from websocket import ABNF


ws = create_connection("ws://192.168.103.244:5000/")

req = dict(
    cmd=1,
    sn=1,
    body='do you love me?',
)

t1 = time.time()

send_buf = json.dumps(req)

print 'len:', len(send_buf)

ws.send(send_buf, ABNF.OPCODE_BINARY)

message = ws.recv()
print 'time past: ', time.time() - t1
print "Received '%r'" % message

rsp = json.loads(message)
print rsp

print 'close begin'
ws.close()
print 'close end'
