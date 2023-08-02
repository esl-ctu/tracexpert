#!/usr/bin/env python

#TODO:
##Otestovat parsování parametrů
##Nastavování properties/subproperties
import sys, time, ctypes, traceback
from PySide6.QtCore import QSharedMemory, QByteArray
import chipwhisperer as cw
import numpy as np

def writeToSHM(line, shm):
    buffer = QByteArray(line)
    size = buffer.size()

    shm.lock()
    _to = memoryview(shm.data()).cast('c')
    _from = buffer
    _to[0:size] = _from[0:size]
    shm.unlock()

def cwToStr(tmp):
    if isinstance(tmp, np.ndarray):
        return np.array2string(tmp)
    else:
        return str(tmp)


#####EXEXUTABLE START######

print("STARTED", flush=True)

PLUGIN_ID = "TraceXpert.NewAE"
NO_CW_ID = 255
FIELD_SEPARATOR = ','
cwDict = dict()

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
    print(line, flush=True, file=sys.stderr) # TODO!!! Remove!!
    ## Test shared memory
    if line.startswith("SMTEST:"):
        line = line[7:]
        line = line.rstrip('\r\n')
        line = "{:016x}".format(len(line)) + line

        writeToSHM(line, shm)

        print("DONE", flush=True)

    ## Detect available CWs
    if line.startswith(str(NO_CW_ID) + ",DETECT_DEVICES"):
        devices = cw.list_devices()
        line = ""
        for i in devices:
            line += i['name']
            line += FIELD_SEPARATOR
            line += i['sn']
            line += '\n'

        line = "{:016x}".format(len(line)) + line
        
        writeToSHM(line, shm)

        print("DONE", flush=True)

    ## Initialize one CW
    if line.startswith("SETUP", 4, 10):
        cwID = line[0:2]
        cwSN = line.split(',')[2].strip()

        if cwID == "" or cwSN == "":
            print("ERROR", flush=True)
            continue

        try:
            cwDict[cwID] = cw.scope(sn=str(cwSN))
        except:
            print("Connection to CW unsuccessful. Is it plugged in? Error: " + traceback.format_exc(), flush=True, file=sys.stderr)
            print("ERROR", flush=True)
            continue

        if cwDict[cwID] != None:
            cwDict[cwID].default_setup()
            print("DONE", flush=True)
        else:
            print("ERROR", flush=True)
            continue

    ## Call a method from the CW package
    if line.startswith("FUNC-", 4, 10):
        cwID = line[0:2]
        scope = cwDict[cwID]
        noParams = False

        functionName = line[9:]
        functionName = functionName.split(FIELD_SEPARATOR, 1)[0]
        functionName = functionName.rstrip('\r\n')
        lineParameters = ""
        try:
        	lineParameters = functionName.split(FIELD_SEPARATOR, 1)[1].strip()
        	lineParameters = lineParameters.rstrip('\r\n')
        except:
        	noParams = True

        if functionName == "":
           print("ERROR", flush=True) 
           print("Invalid Python CW function called (name is empty)", flush=True, file=sys.stderr)
           continue

        try:
            function = getattr(scope, functionName)
        except AttributeError:
            print("ERROR", flush=True) 
            print("Invalid Python CW function called (this method of the CW object does not exist)", flush=True, file=sys.stderr)
            continue

        parameters = []
        numParams = 0
        while (numParams < 10 and lineParameters != "" and (not noParams)):
            parameter = lineParameters.split(FIELD_SEPARATOR, 1)[0]
            parameter = parameters[numParams].rstrip('\r\n')
            if isnumeric(parameter):
            	parameters[numParams] = int(parameter)
            elif isnumeric(parameter[1:] and parameter[0] == "-"):
            	parameters[numParams] = int(parameter) * -1
            elif parameter.lower() == "true":
				parameters[numParams] = True
            elif parameter.lower() == "false":	
            	parameters[numParams] = False
            else:
            	try:
        			tmp = float(parameters)
        			parameters[numParams] = tmp
    			except ValueError:
        			parameters[numParams] = parameter

            lineParameters = functionName.split(FIELD_SEPARATOR, 1)[1]
            numParams += 1

        ret = ""
        try:
            if numParams == 0:
                tmp = function()
                ret = cwToStr(tmp)
            elif numParams == 1:
                tmp = function(parameters[0])
                ret = cwToStr(tmp)
            elif numParams == 2:
                tmp = function(parameters[0], parameters[1])
                ret = cwToStr(tmp)
            elif numParams == 3:
                tmp = function(parameters[0], parameters[1], parameters[2])
                ret = cwToStr(tmp)
            elif numParams == 4:
                tmp = function(parameters[0], parameters[1], parameters[2], parameters[3])
                ret = cwToStr(tmp)
            elif numParams == 5:
                tmp = function(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4])
                ret = cwToStr(tmp)
            elif numParams == 6:
                tmp = function(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5])
                ret = cwToStr(tmp)
            elif numParams == 7:
                tmp = function(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], parameters[6])
                ret = cwToStr(tmp)
            elif numParams == 8:
                tmp = function(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], parameters[6], parameters[7])
                ret = cwToStr(tmp)
            elif numParams == 9:
                tmp = function(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], parameters[6], parameters[7], parameters[8])
                ret = cwToStr(tmp)
            else:
                print("ERROR", flush=True) 
                print("Too many parameters passed to a Python function", flush=True, file=sys.stderr)
        except:
            print("ERROR", flush=True) 
            errorMessage = "The Python CW function raised this exception: " + traceback.format_exc()
            print(errorMessage, flush=True, file=sys.stderr)


        ret = "{:016x}".format(len(ret)) + ret
        
        writeToSHM(ret, shm)

        print("DONE", flush=True)

    if line.startswith("HALT"):
        sys.exit()