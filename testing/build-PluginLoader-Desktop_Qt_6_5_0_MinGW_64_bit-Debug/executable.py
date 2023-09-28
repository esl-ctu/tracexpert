#!/usr/bin/env python

import sys, time, ctypes, traceback
from PySide6.QtCore import QSharedMemory, QByteArray
import chipwhisperer as cw
import numpy as np

##GLOBALS##
PLUGIN_ID = "TraceXpert.NewAE"
NO_CW_ID = 255
FIELD_SEPARATOR = ','
LINE_SEPARATOR = '\n'
shmKey = PLUGIN_ID + "shm2"
shmSize = 1024*1024*1024

##Write to shared memory##
def writeToSHM(line, shm):
    buffer = QByteArray(line)
    size = buffer.size()

    if size > shmSize:
        print("SHM is not large enough. Data would not fit!", flush=True, file=sys.stderr)
        return

    shm.lock()
    _to = memoryview(shm.data()).cast('c')
    _from = buffer
    _to[0:size] = _from[0:size]
    shm.unlock()

##Helper to potentially convert a numpy array to string##
def cwToStr(tmp):
    if isinstance(tmp, np.ndarray):
        return np.array2string(tmp)
    else:
        return str(tmp)

##Handle error when CW is not connected
def sendCWNotConnected():
    cwID = line[0:2]
    cwDict[cwID] = None

    print("NOTCN", flush=True)
    print("Connection to CW unsuccessful. Is it plugged in? Was it initialized? Careful: The CW object was destroyed on this error, please re-intialize the CW scope!", flush=True, file=sys.stderr)

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
    line = line[7:]
    line = line.rstrip('\r\n')
    line = "{:016x}".format(len(line)) + line

    writeToSHM(line, shm)
    print("DONE", flush=True)

def smSet(line):
    global shmSize;
    line = line[7:]
    line = line.rstrip('\r\n')
    try:
        shmSize = int(line)
    except:
        print("Invalid SM size", flush=True, file=sys.stderr)
        print("ERROR", flush=True)


    print("DONE", flush=True)    


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
    print("DONE", flush=True)

##Initialize one CW##
##Takes: cwID, "SETUP", cw_sn
##Outputs: DONE/ERROR
def cwSetup(line, shm, cwDict):
    cwID = line[0:2]
    cwSN = line.split(',')[2].strip()

    if cwID == "" or cwSN == "":
        print("ERROR", flush=True)
        return

    try:
        cwDict[cwID] = cw.scope(sn=str(cwSN))
    except:
        print("Connection to CW unsuccessful. Is it plugged in? Error: " + traceback.format_exc(), flush=True, file=sys.stderr)
        print("ERROR", flush=True)
        return

    if cwDict[cwID] != None:
        try:
            cwDict[cwID].default_setup()
        except:
            print("Connection to CW unsuccessful. Is it plugged in? Error: " + traceback.format_exc(), flush=True, file=sys.stderr)
            print("ERROR", flush=True)
            cwDict[cwID] = None
            return

        print("DONE", flush=True)
    else:
        print("ERROR", flush=True)
        return  

##Call a method from the CW package
##Takes: cwID, "FUNC-", function name, [parameters]
##Outputs: DONE/ERROR, data to shm
def callCwFunc(line, shm, cwDict):
    cwID = line[0:2]
    scope = cwDict[cwID]
    if scope == None:
        sendCWNotConnected(line)
        return

    noParams = False

    functionName = line[9:]
    lineParameters = ""
    splitLine = ""
    try:
        splitLine = functionName.split(FIELD_SEPARATOR, 1)
        functionName = splitLine[0]
        functionName = functionName.rstrip('\r\n')
    except:
        print("ERROR", flush=True) 
        print("Invalid Python CW function called (name is empty) (1)", flush=True, file=sys.stderr)

    try:
        lineParameters = splitLine[1]
        lineParameters = lineParameters.rstrip('\r\n')
    except:
        noParams = True

    if functionName == "":
       print("ERROR", flush=True) 
       print("Invalid Python CW function called (name is empty) (2)", flush=True, file=sys.stderr)
       return

    try:
        function = getattr(scope, functionName)
    except AttributeError:
        print("ERROR", flush=True) 
        print("Invalid Python CW function called (this method of the CW object does not exist)", flush=True, file=sys.stderr)
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
                print("ERROR", flush=True) 
                print("Error processing fucntion parameters", flush=True, file=sys.stderr)
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

##Set or read a scope object parameter
##If No value is provided, read is performed
##Takes: cwID, command name, parameter, [value]
##Outputs: DONE/ERROR, parameter value (to shm)
def cwParam(line, shm, cwDict):
    cwID = line[0:2]
    scope = cwDict[cwID]
    if scope == None:
        sendCWNotConnected(line)
        return

    noValue = False

    params = line[9:]
    paramName = ""
    paramValue = ""
    try:
        paramName = params.split(FIELD_SEPARATOR, 1)[0]
    except:
        print("ERROR", flush=True) 
        print("Invalid Python CW attribute reqested (1)", flush=True, file=sys.stderr) 
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
        param = getattr(scope, paramName)
    except AttributeError:
        print("ERROR", flush=True) 
        print("Invalid Python CW attribute reqested (2)", flush=True, file=sys.stderr)
        return

    if not noValue:
        convParamValue = parseParameter(paramValue)
        setattr(scope, paramName, convParamValue)

    param = getattr(scope, paramName)
    realParamValue = str(param)
    realParamValue = "{:016x}".format(len(realParamValue)) + realParamValue
    writeToSHM(realParamValue, shm)

    print("DONE", flush=True)

##Set or read a scope object subparameter
##If No value is provided, read is performed
##Takes: cwID, command name, parameter, subparameter, [value]
##Outputs: DONE/ERROR, subparameter value (to shm)
def cwSubParam(line, shm, cwDict):
    cwID = line[0:2]
    scope == cwDict[cwID]
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
        print("ERROR", flush=True) 
        print("Invalid Python CW attribute reqested (1)", flush=True, file=sys.stderr) 
        return     
    paramName = paramName.rstrip('\r\n')

    try:
        subParamName = params.split(FIELD_SEPARATOR, 2)[1]
    except:
        print("ERROR", flush=True) 
        print("Invalid Python CW subattribute reqested (1)", flush=True, file=sys.stderr) 
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
        print("ERROR", flush=True) 
        print("Invalid Python CW attribute reqested (2)", flush=True, file=sys.stderr)
        return

    try:
        subParam = getattr(param, subParamName)
    except AttributeError:
        print("ERROR", flush=True) 
        print("Invalid Python CW subattribute reqested (2)", flush=True, file=sys.stderr)
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
        subParamValue = str(subParam)
        subParamValue = "{:016x}".format(len(subParamValue)) + subParamValue
        writeToSHM(subParamValue, shm)

    print("DONE", flush=True)

#####EXEXUTABLE START######
def main():
    print("STARTED", flush=True)

    cwDict = dict()

    shm = QSharedMemory()
    shm.setKey(shmKey)
    shm.attach()
     
    if not shm.isAttached():
        print("Unable to attach SHM.", file=sys.stderr)

    for line in sys.stdin:
        #print(line, flush=True, file=sys.stderr) # TODO!!! Remove!!
        ## Test shared memory
        if line.startswith("SMTEST:"):
            smTest(line, shm)

        if line.startswith("SMSET:"):
            smSet(line)

        ## Detect available CWs
        elif line.startswith(str(NO_CW_ID) + ",DETECT_DEVICES"):
            detectDevices(line, shm)

        ## Initialize one CW
        elif line.startswith("SETUP", 4, 10):
            cwSetup(line, shm, cwDict)

        ## Deinitialize one CW
        elif line.startswith("DEINI", 4, 10):
            cwID = line[0:2]
            cwDict[cwID] = None
            print("DONE", flush=True)

        ## Call a method from the CW package
        elif line.startswith("FUNC-", 4, 10):
            tmpline = line
            try:
                callCwFunc(line, shm, cwDict)
            except(USBError):
                sendCWNotConnected(tmpline)

        ## Set or read a scope parameter
        elif line.startswith("PARA-", 4, 10):
            tmpline = line
            try:
                cwParam(line, shm, cwDict)
            except(USBError):
                sendCWNotConnected(tmpline)

        ## Set or read a scope subparameter 
        elif line.startswith("SPAR-", 4, 10):
            tmpline = line
            try:
                cwSubParam(line, shm, cwDict)
            except(USBError):
                sendCWNotConnected(tmpline)

        ## Exit
        elif line.startswith("HALT"):
            sys.exit()
        
        ## Something went wrong
        else:
            print("ERROR", flush=True) 

if __name__ == "__main__":
    main()
