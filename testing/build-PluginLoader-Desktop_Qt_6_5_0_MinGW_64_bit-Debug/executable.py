#!/usr/bin/env python

import sys, time, ctypes
from PySide6.QtCore import QSharedMemory, QByteArray
import chipwhisperer as cw

def writeToSHM(line, shm):
    buffer = QByteArray(line)
    size = buffer.size()

    shm.lock()
    _to = memoryview(shm.data()).cast('c')
    _from = buffer
    _to[0:size] = _from[0:size]
    shm.unlock()


print("STARTED", flush=True)

PLUGIN_ID = "TraceXpert.NewAE"
NO_CW_ID = 255

shmKey = PLUGIN_ID + "shm2"
shmSize = 1024*1024*1024
shm = QSharedMemory()
shm.setKey(shmKey)
shm.attach()
 
if not shm.isAttached():
    print("Had to create SHM, the program may not work correctly.", file=sys.stderr)
    if not shm.create(shmSize):
        print("Unable to attach or even create SHM.", file=sys.stderr)

for line in sys.stdin:
    print(line, flush=True, file=sys.stderr)
    if line.startswith("SMTEST:"):
        line = line[7:]
        line = line.rstrip('\r\n')
        line = "{:016x}".format(len(line)) + line

        writeToSHM(line, shm)

        print("DONE", flush=True)
    if line.startswith(str(NO_CW_ID) + ",DETECT_DEVICES"):
        print("DETECTDEVICES", flush=True, file=sys.stderr)
        devices = cw.list_devices()
        line = ""
        for i in devices:
            line += i['name']
            line += ','
            line += i['sn']
            line += '\n'

        line = "{:016x}".format(len(line)) + line

        writeToSHM(line, shm)

        print("DONE", flush=True)

    if line.startswith("HALT"):
        sys.exit()