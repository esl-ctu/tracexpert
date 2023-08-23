#!/usr/bin/env python

#TODO:
##Otestovat parsování parametrů
##Nastavování properties/subproperties
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

##Test shared memory##
def smTest(line, shm):
    line = line[7:]
    line = line.rstrip('\r\n')
    line = "{:016x}".format(len(line)) + line

    writeToSHM(line, shm)
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
        cwDict[cwID].default_setup()
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
    noParams = False

    functionName = line[9:]
    try:
        functionName = functionName.split(FIELD_SEPARATOR, 1)[0]
        functionName = functionName.rstrip('\r\n')
    except:
        print("ERROR", flush=True) 
        print("Invalid Python CW function called (name is empty) (1)", flush=True, file=sys.stderr)

    lineParameters = line[9:]
    try:
        lineParameters = lineParameters.split(FIELD_SEPARATOR, 1)[1].strip()
        lineParameters = lineParameters.rstrip('\r\n')
    except:
        noParams = True
    print(noParams, flush=True, file=sys.stderr) # TODO!!! Remove!!

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
        print("a", flush=True, file=sys.stderr) # TODO!!! Remove!!
        try:
            parameter = lineParameters.split(FIELD_SEPARATOR, 1)[0]
            parameter = parameters[numParams].rstrip('\r\n')
        except:
            try:
                parameter = lineParameters.split(LINE_SEPARATOR, 1)[0]
                parameter = parameters[numParams].rstrip('\r\n')
            except:
                print("ERROR", flush=True) 
                print("Error processing fucntion parameters", flush=True, file=sys.stderr)
                return
        if parameter.isnumeric():
            parameters[numParams] = int(parameter)
        elif (parameter[1:]).isnumeric() and parameter[0] == "-":
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
        
        numParams += 1

        try:
            lineParameters = functionName.split(FIELD_SEPARATOR, 1)[1]
        except:
            break
        
    ret = ""
    print(numParams, flush=True, file=sys.stderr) # TODO!!! Remove!!
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
    noValue = False

    params = line[9:]
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
    scope = cwDict[cwID]
    pass
    #TODO

#####EXEXUTABLE START######
def main():
    print("STARTED", flush=True)

    cwDict = dict()

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
            smTest(line, shm)

        ## Detect available CWs
        elif line.startswith(str(NO_CW_ID) + ",DETECT_DEVICES"):
            detectDevices(line, shm)

        ## Initialize one CW
        elif line.startswith("SETUP", 4, 10):
            cwSetup(line, shm, cwDict)

        ## Call a method from the CW package
        elif line.startswith("FUNC-", 4, 10):
            callCwFunc(line, shm, cwDict)

        ## Set or read a scope parameter
        elif line.startswith("PARA-", 4, 10):
            cwParam(line, shm, cwDict)

        ## Set or read a scope subparameter 
        elif line.startswith("SPAR-", 4, 10):
            cwSubParam(line, shm, cwDict)

        ## Exit
        elif line.startswith("HALT"):
            sys.exit()
        
        ## Something went wrong
        else:
            print("ERROR", flush=True) 

if __name__ == "__main__":
    main()
