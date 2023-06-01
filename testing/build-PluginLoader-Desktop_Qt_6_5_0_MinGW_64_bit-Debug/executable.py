#!/usr/bin/env python

import sys, time, ctypes
from PySide6.QtCore import QSharedMemory, QBuffer, QIODeviceBase, QDataStream, QByteArray

print("STARTED", flush=True)

PLUGIN_ID = "TraceXpert.NewAE"
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
    if line.startswith("SMTEST:"):
        print("SMTEST", file=sys.stderr)
        line = line[7:]
        line = line.rstrip('\r\n')
        line = "{:016x}".format(len(line)) + line

        buffer = QByteArray(line)
        #buffer = QBuffer()
        #buffer.open(QIODeviceBase.WriteOnly)
        #out = QDataStream(buffer)
        #out << line.encode(encoding = 'ascii')
        #buffer.close()
        size = buffer.size()


        shm.lock()
        _to = memoryview(shm.data()).cast('c')
        _from = buffer#.data().data()
        print(_from[0:size], file=sys.stderr)
        _to[0:size] = _from[0:size]
        shm.unlock()

        

        print("DONE", flush=True)
    if line.startswith("DETECT_DEVICES"):
        print("DETECTDEVICES", flush=True)
    if line.startswith("HALT"):
        sys.exit()