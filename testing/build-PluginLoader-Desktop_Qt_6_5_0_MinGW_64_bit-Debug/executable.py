#!/usr/bin/env python
#Todo: Stopping threads

import sys, time, ctypes, traceback, struct
from PySide6.QtCore import QSharedMemory, QByteArray, QMutex
import chipwhisperer as cw
import numpy as np
import logging
from threading import Thread
from queue import Queue
from ctypes import c_uint8 as uint16_t
from ctypes import c_double as double

##GLOBALS##
PLUGIN_ID = "TraceXpert.NewAE"
NO_CW_ID = 255
FIELD_SEPARATOR = ','
LINE_SEPARATOR = '\n'
shmKey = PLUGIN_ID + "shm2"
#shm = QSharedMemory()
stdoutMutex = QMutex()
stderrMutex = QMutex()



def printToStdout(data, asIODevice, cwID):
    toPrint = data
    if asIODevice == True:
        data += ",IO,"
    else:
        data += ",SC,"
    data += str(cwID)
    #printToStderr(data) #TODO remove

    stdoutMutex.lock()
    print(data, flush=True)
    stdoutMutex.unlock()


def printToStderr(data):
    stderrMutex.lock()
    print(data, flush=True, file=sys.stderr)
    stderrMutex.unlock()


##Write to shared memory##
def writeToSHM(line, shm):
    shm.lock()
    _to = memoryview(shm.data()).cast('c')
    if isinstance(line, np.ndarray):
        if isinstance(line[0], np.int16):
            tmpLen = "{:016x}".format(line.size * 2)
            leng = bytes(tmpLen, 'ascii')
            buffer = QByteArray(leng)
            _from = memoryview(buffer).cast('c')
            _to[0:16] = _from[0:16]

            _to = memoryview(shm.data()).cast('h')
            _from = line #memoryview(line).cast('h')
            size = _from.size
            if size + 16 > shm.size():
                printToStderr("SHM is not large enough. Data would not fit!")
                return
            _to[8:(size+8)] = line[0:size]
        else:
            tmpLen = "{:016x}".format(line.size * 8)
            leng = bytes(tmpLen, 'ascii')
            buffer = QByteArray(leng)
            _from = memoryview(buffer).cast('c')
            _to[0:16] = _from[0:16]

            _to = memoryview(shm.data()).cast('d')
            _from = line #memoryview(line).cast('d')
            size = _from.size
            if size + 16 > shm.size():
                printToStderr("SHM is not large enough. Data would not fit!")
                return
            _to[2:(size+2)] = _from[0:size]
    else:    
        buffer = QByteArray(line)  
        _from = memoryview(buffer).cast('c')
        size = buffer.size() 
        if size > shm.size():
            printToStderr("SHM is not large enough. Data would not fit!")
            return
        _to[0:size] = _from[0:size]
    shm.unlock()

##Helper to potentially convert a numpy array to string##
def cwToStr(tmp):
    
    if isinstance(tmp, np.ndarray):
        return tmp
    else:
        return str(tmp)

##Handle error when CW is not connected
def sendCWNotConnected(line):
    cwID = line[0:3]
    cwDict[cwID] = None
    targetDict[cwID] = None

    try:
        if line[4] == 't' and line[5] == '-':
            printToStdout("NOTCN", True, cwID)
        else:
            printToStdout("NOTCN", False, cwID)
    except:
        printToStdout("NOTCN", False, cwID)
    printToStderr("Connection to CW unsuccessful. Is it plugged in? Was it initialized? Careful: The CW and taget objects were destroyed on this error, please re-intialize the CW scope/target!")

##Helper for pasing parameters from a string input
## Takes: Parameter in a form of a string
## Returns: Correctly typed parameter
def parseParameter(parameter):
    if parameter.isnumeric():
        return int(parameter)
    elif (parameter[1:]).isnumeric() and parameter[0] == "-":
        return int(parameter) * -1
    elif parameter.lower() == "true":
        return True
    elif parameter.lower() == "false":  
        return False
    else:
        try:
            tmp = float(parameter)
            return tmp
        except ValueError:
            return parameter

##Test shared memory##
def smTest(line, shm):
    cwID = line[0:3]
    asTarget = False
    if line[4] == 't' and line[5] == '-':
        asTarget = True
        line = line[13:]
    else:
        line = line[11:]

    line = line.rstrip('\r\n')

    line = "{:016x}".format(len(line)) + line

    writeToSHM(line, shm)
    if asTarget:
        printToStdout("DONE", True, cwID)
    else:
        printToStdout("DONE", False, cwID)

def smSet(line, shm):
    shmSize = 0

    cwID = line[0:3]
    asTarget = False
    if line[4] == 't' and line[5] == '-':
        asTarget = True
        line = line[12:]
    else:
        line = line[10:]

    line = line.rstrip('\r\n')
    try:
        shmSize = int(line)
    except:
        printToStderr("Invalid SM size")
        printToStderr(line)
        if asTarget:
            printToStdout("ERROR", True, cwID)
        else:
            printToStdout("ERROR", False, cwID)

    if asTarget:
        shm.setKey(shmKey + cwID + "t")
    else:
        shm.setKey(shmKey + cwID)

    shm.attach()

    if not shm.isAttached():
        printToStderr("Unable to attach SHM.")
    if asTarget:
        printToStdout("DONE", True, cwID)
    else:
        printToStdout("DONE", False, cwID)    


##Detect connected CW devices##
##Takes: <nothing>
##Outputs: DONE, list of CWs (to shm)
def detectDevices(line, shm):
    devices = cw.list_devices()
    line = ""
    for i in devices:
        line += i['name']
        line += FIELD_SEPARATOR
        line += i['sn']
        line += '\n'

    line = "{:016x}".format(len(line)) + line
        
    writeToSHM(line, shm)
    printToStdout("DONE", False, NO_CW_ID)

##Initialize one CW##
##Takes: cwID, "SETUP", cw_sn
##Outputs: DONE/ERROR
def cwSetup(line, shm, cwDict):
    cwID = line[0:3]
    cwSN = line.split(',')[2].strip()

    if cwID == "" or cwSN == "":
        printToStdout("ERROR", False, cwID)
        printToStderr("No id or sn")
        return False

    try:
        cwDict[cwID] = cw.scope(sn=str(cwSN))
    except:
        printToStderr("Connection to CW unsuccessful. Is it plugged in? Error: " + traceback.format_exc())
        printToStdout("ERROR", False, cwID)
        return False

    if cwDict[cwID] != None:
        try:
            cwDict[cwID].default_setup()
        except:
            printToStderr("Connection to CW unsuccessful. Is it plugged in? Error: " + traceback.format_exc())
            printToStdout("ERROR", False, cwID)
            cwDict[cwID] = None
            return False

        printToStdout("DONE", False, cwID)
        return True
    else:
        printToStdout("ERROR", False, cwID)
        printToStderr("CW does not exist)")
        return False

def targetSetup(line, shm, targetDict, sc):
    cwID = line[0:3]
    if cwID == "":
        printToStdout("ERROR", True, cwID)
        printToStderr("No id")
        return False

    try:
        targetDict[cwID] = cw.target(sc)
    except:
        printToStderr("Connection to target unsuccessful. Error: " + traceback.format_exc())
        printToStdout("ERROR", True, cwID)
        return False

    if targetDict[cwID] == None:
        printToStdout("ERROR", True, cwID)
        printToStderr("CW does not exist)")
        return False

    printToStdout("DONE", True, cwID)
    return True

##Call a method on an object from the CW package
##Takes: cwID, "FUNO-", obejct name, function name
##Outputs: DONE/ERROR, data to shm
def callCwFuncOnAnObject(line, shm, cwDict):
    cwID = line[0:3]
    scope = cwDict[cwID]
    if scope == None:
        sendCWNotConnected(line)
        return

    #Split to object name and function name
    objectAndFunctionNames = line[9:]
    objectName = ""
    functionName = ""
    splitLine = ""
    try:
        splitLine = objectAndFunctionNames.split(FIELD_SEPARATOR, 1)
        objectName = splitLine[0]
        objectName = objectName.rstrip('\r\n')
        functionName = splitLine[1]
        functionName = functionName.rstrip('\r\n')
    except:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW function called or it was called on an invalid object (one of the names is probably empty)")

    try:
        subObject = getattr(scope, objectName)
    except AttributeError:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW object specified  (this object of the CW object does not exist)")
        return

    try:
        function = getattr(subObject, functionName)
    except AttributeError:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW function called (this method of the specified subobject of the CW object does not exist)")
        return

    ret = ""
    try:
        tmp = function()
        ret = cwToStr(tmp)
    except:
        printToStdout("ERROR", False, cwID) 
        errorMessage = "The Python CW function raised this exception: " + traceback.format_exc()
        printToStderr(errorMessage)

    if isinstance(ret, bytes):
        #tmpLen = "{:016x}".format(len(ret))
        #ret = bytes(tmpLen, 'ascii') + ret
        pass
    else:
        ret = "{:016x}".format(len(ret)) + ret
    
    writeToSHM(ret, shm)

    printToStdout("DONE", False, cwID)


##Call a method from the CW package
##Takes: cwID, "FUNC-"/"T-FUNC-", function name, [parameters]
##Outputs: DONE/ERROR, data to shm
def callCwFunc(line, shm, dct):
    cwID = line[0:3]
    asTarget = False
    if line[4] == 't' and line[5] == '-':
        asTarget = True
        target = dct[cwID]
        if target == None:
            sendCWNotConnected(line)
            return
    else:
        scope = dct[cwID]
        if scope == None:
            sendCWNotConnected(line)
            return

    noParams = False

    functionName = ""
    if asTarget == True:
        functionName = line[11:]
    else:
        functionName = line[9:]
    lineParameters = ""
    splitLine = ""
    try:
        splitLine = functionName.split(FIELD_SEPARATOR, 1)
        functionName = splitLine[0]
        functionName = functionName.rstrip('\r\n')
    except:
        if asTarget == True:
            printToStdout("ERROR", True, cwID)
        else:
            printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW function called (name is empty) (1)")

    try:
        lineParameters = splitLine[1]
        lineParameters = lineParameters.rstrip('\r\n')
    except:
        noParams = True

    if functionName == "":
        if asTarget == True:
            printToStdout("ERROR", True, cwID) 
        else:
            printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW function called (name is empty) (2)")
        return

    try:
        if asTarget == True:
            function = getattr(target, functionName)
        else:
            function = getattr(scope, functionName)
    except AttributeError:
        if asTarget == True:
            printToStdout("ERROR", True, cwID)
        else:
            printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW/target function called (this method of the CW/target object does not exist)")
        return

    parameters = [None] * 10
    numParams = 0
    while (numParams < 10 and lineParameters != "" and (not noParams)):
        try:
            parameter = lineParameters.split(FIELD_SEPARATOR, 1)[0]
            parameter = parameter.rstrip('\r\n')
        except:
            try:
                parameter = lineParameters.split(LINE_SEPARATOR, 1)[0]
                parameter = parameters[numParams].rstrip('\r\n')
            except:
                if asTarget == True:
                    printToStdout("ERROR", True, cwID) 
                else:
                    printToStdout("ERROR", False, cwID) 
                printToStderr("Error processing fucntion parameters")
                return
        
        parameters[numParams] = parseParameter(parameter)
        numParams += 1

        try:
            lineParameters = lineParameters.split(FIELD_SEPARATOR, 1)[1]
        except:
            break
        
    ret = ""
    try:
        if numParams == 0:
            tmp = function()
            ret = cwToStr(tmp)
        elif numParams == 1:
            '''if functionName == "get_last_trace":
                shm.lock()
                numSamples = 5000 #todo
                if parameters[0] == True: #traces as ints
                    _to = memoryview(shm.data()).cast('c')
                    tmpLen = "{:016x}".format(numSamples * 2)
                    leng = bytes(tmpLen, 'ascii')
                    _from = QByteArray(leng)
                    _to[0:16] = _from[0:16]
                    _to = memoryview(shm.data()).cast('h')
                    result = np.ndarray(shape=(numSamples,), dtype=np.int16, buffer=_to[16:])
                    result = function(parameters[0])
                else: #traces as doubles
                    _to = memoryview(shm.data()).cast('c')
                    tmpLen = "{:016x}".format(numSamples * 8)
                    leng = bytes(tmpLen, 'ascii')
                    _from = QByteArray(leng)
                    _to[0:16] = _from[0:16]
                    _to = memoryview(shm.data()).cast('d')
                    result = np.ndarray(shape=(numSamples,), dtype=np.double, buffer=_to[16:])
                    result = function(parameters[0])
                shm.unlock()
                stri = ""
                for i in range(0,5000):
                    #tmp[i] = i
                    stri += str(result[i])
                    stri += "\r\n"
                printToStderr(stri)
                printToStdout("DONE", False, cwID)
                return
            else:
                tmp = function(parameters[0])
                ret = cwToStr(tmp)'''
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
            if asTarget == True:
                printToStdout("ERROR", True, cwID) 
            else:
                printToStdout("ERROR", False, cwID) 
            printToStderr("Too many parameters passed to a Python function")
    except:
        if functionName != "capture":
            if asTarget == True:
                printToStdout("ERROR", True, cwID) 
            else:
                printToStdout("ERROR", False, cwID) 
            errorMessage = "The Python CW function raised this exception: " + traceback.format_exc()
            printToStderr(errorMessage)
        else:
            pass

    if isinstance(ret, np.ndarray):
        pass
        #tmpLen = "{:016x}".format(len(ret))
        #ret = bytes(tmpLen, 'ascii') + ret
    else:
        ret = "{:016x}".format(len(ret)) + ret
    
    writeToSHM(ret, shm)

    if asTarget == True:
        printToStdout("DONE", True, cwID) 
    else:
        printToStdout("DONE", False, cwID)

##Set or read a scope object parameter
##If No value is provided, read is performed
##Takes: cwID, command name, parameter, [value]
##Outputs: DONE/ERROR, parameter value (to shm)
def cwParam(line, shm, dct):
    cwID = line[0:3]
    asTarget = False
    if line[4] == 't' and line[5] == '-':
        asTarget = True
        target = dct[cwID]
        if target == None:
            sendCWNotConnected(line)
            return
    else:
        scope = dct[cwID]
        if scope == None:
            sendCWNotConnected(line)
            return

    noValue = False

    if asTarget == True:
        params = line[11:]
    else:
        params = line[9:]
    
    paramName = ""
    paramValue = ""
    try:
        paramName = params.split(FIELD_SEPARATOR, 1)[0]
    except:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW attribute reqested (1)") 
        return     
    paramName = paramName.rstrip('\r\n')

    try:
        paramValue = params.split(FIELD_SEPARATOR, 1)[1]
    except:
        noValue = True

    if not noValue:
        paramValue = paramValue.rstrip('\r\n')
        if len(paramValue) == 0:
            noValue = True

    try:
        if asTarget == True:
            param = getattr(target, paramName)
        else:
            param = getattr(scope, paramName)
    except AttributeError:
        if asTarget == True:
            printToStdout("ERROR", True, cwID)
        else:
            printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW attribute reqested (2)")
        return

    if not noValue:
        convParamValue = parseParameter(paramValue)
        if asTarget == True:
            setattr(target, paramName, convParamValue)
        else:
            setattr(scope, paramName, convParamValue)

    realParamValue = str(param)
    realParamValue = "{:016x}".format(len(realParamValue)) + realParamValue
    writeToSHM(realParamValue, shm)

    if asTarget == True:
        printToStdout("DONE", True, cwID)
    else:
        printToStdout("DONE", False, cwID)

##Set or read a scope object subparameter
##If No value is provided, read is performed
##Takes: cwID, command name, parameter, subparameter, [value]
##Outputs: DONE/ERROR, subparameter value (to shm)
def cwSubParam(line, shm, cwDict):
    cwID = line[0:3]
    scope = cwDict[cwID]
    if scope == None:
        sendCWNotConnected(line)
        return

    noValue = False
    
    params = line[9:]
    paramName = ""
    subParamName = ""
    subParamValue = ""
    try:
        paramName = params.split(FIELD_SEPARATOR, 1)[0]
    except:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW attribute reqested (1)") 
        return     
    paramName = paramName.rstrip('\r\n')

    try:
        subParamName = params.split(FIELD_SEPARATOR, 2)[1]
    except:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW subattribute reqested (1)") 
        return     
    subParamName = subParamName.rstrip('\r\n')

    try:
        subParamValue = params.split(FIELD_SEPARATOR, 2)[2]
    except:
        noValue = True

    if not noValue:
        subParamValue = subParamValue.rstrip('\r\n')
        if len(subParamValue) == 0:
            noValue = True

    try:
        param = getattr(scope, paramName)
    except AttributeError:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW attribute reqested (2)")
        return

    try:
        subParam = getattr(param, subParamName)
    except AttributeError:
        printToStdout("ERROR", False, cwID) 
        printToStderr("Invalid Python CW subattribute reqested (2)")
        return

    if not noValue:
        convSubParamValue = parseParameter(subParamValue)
        setattr(getattr(scope, paramName), subParamName, convSubParamValue)

        param = getattr(scope, paramName)
        subParam = getattr(param, subParamName)
        realSubParamValue = str(subParam)
        realSubParamValue = "{:016x}".format(len(realSubParamValue)) + realSubParamValue
        writeToSHM(realSubParamValue, shm)
    else:
        subParamValue = ""
        if (isinstance(subParam, bytearray)):
            subParamValue = str(int.from_bytes(subParam, byteorder='big'))
        else:
            subParamValue = str(subParam)
        subParamValue = "{:016x}".format(len(subParamValue)) + subParamValue
        writeToSHM(subParamValue, shm)

    printToStdout("DONE", False, cwID)

def consumerCw(queue, cwShmDict, cwDict):
    while True:
        line = queue.get()

        if line == "DIE":
            break

        ## Call a method from the CW package
        if line.startswith("FUNC-", 4, 10):
            cwID = line[0:3]
            tmpline = line
            try:
                callCwFunc(line.lower(), cwShmDict[cwID], cwDict)
            except(USBError):
                sendCWNotConnected(tmpline)

        ## Call a method on an object from the CW package
        elif line.startswith("FUNO-", 4, 10):
            cwID = line[0:3]
            tmpline = line
            try:
                callCwFuncOnAnObject(line.lower(), cwShmDict[cwID], cwDict)
            except(USBError):
                sendCWNotConnected(tmpline)

        ## Set or read a scope parameter
        elif line.startswith("PARA-", 4, 10):
            cwID = line[0:3]
            tmpline = line
            try:
                cwParam(line.lower(), cwShmDict[cwID], cwDict)
            except(USBError):
                sendCWNotConnected(tmpline)

        ## Set or read a scope subparameter 
        elif line.startswith("SPAR-", 4, 10):
            cwID = line[0:3]
            tmpline = line
            try:
                cwSubParam(line.lower(), cwShmDict[cwID], cwDict)
            except(USBError):
                sendCWNotConnected(tmpline)

def consumerTarget(queue, targetShmDict, targetDict):
     while True:
        line = queue.get()

        if line == "DIE":
            break

        ## Call a method from the CW package
        if line.startswith("T-FUNC-", 4, 12):
            cwID = line[0:3]
            tmpline = line
            try:
                callCwFunc(line.lower(), targetShmDict[cwID], targetDict)
            except(USBError):
                sendCWNotConnected(tmpline)

        ## Set or read a scope parameter
        elif line.startswith("T-PARA-", 4, 12):
            cwID = line[0:3]
            tmpline = line
            try:
                cwParam(line.lower(), targetShmDict[cwID], targetDict)
            except(USBError):
                sendCWNotConnected(tmpline)
        

#####EXEXUTABLE START######
def main():
    print("STARTED", flush=True)
    cw.scope_logger.setLevel(logging.ERROR)

    cwDict = dict()
    targetDict = dict()

    cwShmDict = dict()
    targetShmDict = dict()

    cwQueueDict = dict()
    targetQueueDict = dict()

    cwConsumerDict = dict()
    targetConsumerDict = dict()

    for line in sys.stdin:
        #print(line, flush=True, file=sys.stderr) # TODO!!! Remove!!
        if line == "\r" or line == "\n" or line == "\r\n":
            continue

        ## Test shared memory
        if line.startswith("SMTEST:", 4, 12):
            cwID = line[0:3]
            smTest(line.lower(), cwShmDict[cwID])

        elif line.startswith("SMSET:", 4, 11):
            cwID = line[0:3]
            cwShmDict[cwID] = QSharedMemory()
            smSet(line.lower(), cwShmDict[cwID])

        ## Test shared memory for target
        elif line.startswith("T-SMTEST:", 4, 14):
            cwID = line[0:3]
            smTest(line.lower(), targetShmDict[cwID])

        elif line.startswith("T-SMSET:", 4, 13):
            cwID = line[0:3]
            targetShmDict[cwID] = QSharedMemory()
            smSet(line.lower(), targetShmDict[cwID])

        ## Detect available CWs
        elif line.startswith(str(NO_CW_ID) + ",DETECT_DEVICES"):
            detectDevices(line.lower(), cwShmDict[str(NO_CW_ID)])

        ## Initialize one CW
        elif line.startswith("SETUP", 4, 10):
            cwID = line[0:3]
            if cwSetup(line.lower(), cwShmDict[cwID], cwDict):
                cwQueueDict[cwID] = Queue()
                cwConsumerDict[cwID] = Thread(target=consumerCw, args=(cwQueueDict[cwID], cwShmDict, cwDict))
                cwConsumerDict[cwID].start()

        ## Initialize one CW taget
        elif line.startswith("T-SETUP", 4, 12):
            cwID = line[0:3]
            if targetSetup(line.lower(), targetShmDict[cwID], targetDict, cwDict[cwID]):
                targetQueueDict[cwID] = Queue()
                targetConsumerDict[cwID] = Thread(target=consumerTarget, args=(targetQueueDict[cwID], targetShmDict, targetDict))
                targetConsumerDict[cwID].start()

        ## Deinitialize one CW
        elif line.startswith("DEINI", 4, 10):
            cwID = line[0:3]
            try:
                cwDict[cwID].dis()
            except:
                pass
            cwDict[cwID] = None
            cwQueueDict[cwID].put("DIE")
            cwConsumerDict[cwID].join()
            printToStdout("DONE", False, cwID)

        ## Deinitialize one CW target
        elif line.startswith("T-DEINI", 4, 12):
            targetID = line[0:3]
            try:
                targetDict[cwID].dis()
            except:
                pass
            targetDict[cwID] = None
            targetQueueDict[cwID].put("DIE")
            targetConsumerDict[cwID].join()
            printToStdout("DONE", True, cwID)

        ## Call a method from the CW package
        elif line.startswith("FUNC-", 4, 10):
            cwID = line[0:3]
            cwQueueDict[cwID].put(line)

        ## Call a method from the CW target package
        elif line.startswith("T-FUNC-", 4, 12):
            cwID = line[0:3]
            targetQueueDict[cwID].put(line)

        ## Call a method on an object from the CW package
        elif line.startswith("FUNO-", 4, 10):
            cwID = line[0:3]
            cwQueueDict[cwID].put(line)

        ## Set or read a scope parameter
        elif line.startswith("PARA-", 4, 10):
            cwID = line[0:3]
            cwQueueDict[cwID].put(line)

        ## Set or read a target parameter
        elif line.startswith("T-PARA-", 4, 12):
            cwID = line[0:3]
            targetQueueDict[cwID].put(line)

        ## Set or read a scope subparameter 
        elif line.startswith("SPAR-", 4, 10):
            cwID = line[0:3]
            cwQueueDict[cwID].put(line)

        ## Exit
        elif line.startswith("HALT"):
            sys.exit()
        
        ## Something went wrong
        else:
            printToStdout("ERROR", False, NO_CW_ID) 
            printToStderr("The following was evaluated as an error: " + line)

if __name__ == "__main__":
    main()
