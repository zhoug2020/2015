import numpy
import pylab
import re
import json
import sys
import os

import time
import datetime

import threading
import math


class  ERetVal:
    Success = 0
    ErrNotCalibrated = 1
    WarnOutOfRangeLow = 2
    WarnOutOfRangeHigh = 3
    WarnCalibrationLimitExceeded  = 4

class LinReg_state:
    N = 0
    temperature_low = 0.0
    temperature_high = 0.0
    S = 0.0
    Sx = 0.0
    Sy = 0.0
    Sxx = 0.0
    Sxy = 0.0

class GyroZroTempCalLinReg:
    state = LinReg_state()
    solution_valid = False
    a = 0.0
    b = 0.0
    var_a = 0.0
    var_b = 0.0
    cov_ab = 0.0
    r_ab = 0.0

    def reset(self):
        self.state.N = 0
        self.state.S = 0.0
        self.state.Sx = 0.0
        self.state.Sy = 0.0
        self.state.Sxx = 0.0
        self.state.Sxy = 0.0

    def __init__(self):
        self.reset()

    def isfinit(x):
        if (x <= 1.7976931348623158e+308  and x > - 1.7976931348623158e+308):
            return True
        else:
            return False

    def calcDerivedState(self):
        delta = self.state.S * self.state.Sxx - self.state.Sx * self.state.Sx
        if(delta != 0.0):
            self.solution_valid = True
            self.a = (self.state.Sxx * self.state.Sy - self.state.Sx * self.state.Sxy) / delta
            self.b = (self.state.S * self.state.Sxy - self.state.Sx * self.state.Sy) / delta
            self.var_a = self.state.Sxx / delta
            self.var_b = self.state.S / delta
            self.cov_ab = - self.state.Sx / delta
            self.r_ab = - self.state.Sx / math.sqrt(self.state.S * self.state.Sxx)
        else:
            self.solution_valid = False


    def addZroMeasurement(self, temperature_sensor_reading, zro_angular_rate_reading):
        retval = ERetVal.Success
        x = temperature_sensor_reading
        y = zro_angular_rate_reading
        sigma_y = 1.0
        inv_var_y = 1.0
        if(self.state.N < 0xffffffff):
            if(self.state.N == 0):
                self.state.temperature_low = temperature_sensor_reading
                self.state.temperature_high = temperature_sensor_reading
            else:
                if(self.state.temperature_low > temperature_sensor_reading):
                    self.state.temperature_low = temperature_sensor_reading
                if(self.state.temperature_high < temperature_sensor_reading):
                    self.state.temperature_high = temperature_sensor_reading
            self.state.N = self.state.N + 1
            self.state.S = self.state.S + inv_var_y
            self.state.Sx = self.state.Sx + x * inv_var_y
            self.state.Sy = self.state.Sy + y* inv_var_y
            self.state.Sxx = self.state.Sxx + x*x*inv_var_y
            self.state.Sxy = self.state.Sxy + x * y * inv_var_y

            if((not GyroZroTempCalLinReg.isfinit(self.state.S)) or
                (not GyroZroTempCalLinReg.isfinit(self.state.Sx)) or
                (not GyroZroTempCalLinReg.isfinit(self.state.Sy)) or
                (not GyroZroTempCalLinReg.isfinit(self.state.Sxx)) or
                (not GyroZroTempCalLinReg.isfinit(self.state.Sxy))):

                self.reset()
                self.state.N = self.state.N + 1
                self.state.S = self.state.S + inv_var_y
                self.state.Sx = self.state.Sx + x * inv_var_y
                self.state.Sy = self.state.Sy + y* inv_var_y
                self.state.Sxx = self.state.Sxx + x*x*inv_var_y
                self.state.Sxy = self.state.Sxy + x * y * inv_var_y

            self.calcDerivedState()

            retval = ERetVal.Success

        else:
            retval = ERetVal.WarnCalibrationLimitExceeded

        return retval

    def estimateZro(self, temperature_sensor_reading, zro_angular_rate_estimated):
        retval = ERetVal.ErrNotCalibrated
        if(self.solution_valid):
            zro_angular_rate_estimated[0] = (float)(self.a + self.b * temperature_sensor_reading)
        if(self.state.temperature_low > temperature_sensor_reading):
            retval = ERetVal.WarnOutOfRangeLow
        elif(self.state.temperature_high < temperature_sensor_reading):
            retval = ERetVal.WarnOutOfRangeHigh
        else:
            retval = ERetVal.Success

        return retval

    def getCalibrationRange(self, temperature_low, temperature_high):
        retVal = False
        if(self.state.N):
            temperature_low = self.state.temperature_low
            temperature_high = self.state.temperature_high
            retVal = True
        return retVal

    def getState(self,state):
        return self.state

    def restoreState(self, state):
        self.state = state
        self.calcDerivedState()
        return True

    def increaseUncertainty(self, std_factor):
        retval = False
        if(std_factor > 1):
            inv_var_factor = 1 / (std_factor* std_factor)

            self.state.S = self.state.S * inv_var_factor
            self.state.Sx = self.state.Sx *inv_var_factor
            self.state.Sy = self.state.Sy * inv_var_factor
            self.state.Sxx = self.state.Sxx * inv_var_factor
            self.state.Sxy = self.state.Sxy * inv_var_factor
            retval = True

        self.calcDerivedState()
        return retval

    def geta(self):
        return self.a

    def getb(self):
        return self.b

class helpFunc:
    def  deal_string(strVal, splitstr):
        results = []
        for item in strVal.split(splitstr):
            if not item.isspace() and len(item) > 0:
                # print(line)
                results.append(item)
        return results

    def regSearch(content, regStr):
        patten = re.compile(regStr)
        result = re.findall(patten, content)
        if len(result) == 0:
            # print(content)
            pass
        return ("".join(result))


    def getPathName(path):
        if len(path) == 0:
            currentPath = os.getcwd()
        else:
            currentPath = path
        if currentPath[:-1] != os.path.sep:
            currentPath += os.path.sep
        return currentPath

    def getlogfiles(path):
        try:
            log_files= []
            currentPath = helpFunc.getPathName(path)
            files = os.listdir(currentPath)
            files.sort(key= lambda x : os.stat(currentPath + x).st_mtime)
            for name in files:
                #print("%s %s" %(name, time.ctime(os.stat(currentPath + name).st_mtime)))
                if os.path.isfile(currentPath + name) and re.search("(^data).*log$", name, re.IGNORECASE) is not None:
                    log_files.append( currentPath + name)

            filename = log_files[-1]
            return filename
            pass
        except Exception as err:
            print(err)

    def nowtime():
        now = int(time.time())
        return now

class LogParser:

    def parselog(self, logfileName):

        if(len(logfileName) == 0): #empty
            logfileName = helpFunc.getlogfiles("")
        elif os.path.isdir(logfileName):
            logfileName = helpFunc.getlogfiles(logfileName)
            print(logfileName)
        resultList = []
        try:
            tempArry = []
            dataArry = []
            biasArry = []
            for line in open(logfileName):
                items = helpFunc.deal_string(line, "\t")
                if(len(items) == 2):
                    tempArry.append(float(items[0]))
                    dataArry.append(float(items[1]))

            resultList.append(tempArry)
            resultList.append(dataArry)
            state = LinReg_state()
            state.temperature_high = 1000
            state.temperature_low = -300
            gyroZeroCal = GyroZroTempCalLinReg()

            for i, val in enumerate(dataArry):
                ret = gyroZeroCal.addZroMeasurement(tempArry[i], val)
                if(ret != ERetVal.Success):
                    gyroZeroCal.reset()
                    gyroZeroCal.addZroMeasurement(tempArry[i], val)

                biasValRefArr = [0.0]

                gyroZeroCal.estimateZro(tempArry[i], biasValRefArr)
                #print("%f, %f", (tempArry[i],biasValRefArr[0]))
                print("a:%f, b:%f", (gyroZeroCal.geta(), gyroZeroCal.getb()))
                biasArry.append(biasValRefArr[0])


            resultList.append(biasArry)

            self.logPlot(resultList)
            resultList.clear()

        except Exception as err:
            print(err)


    def logPlot(self, resultList):

        pylab.figure()
        pylab.plot(resultList[1],  label = 'gyrodata')
        pylab.plot(resultList[2], label= 'bias')
        pylab.legend()

        pylab.show()
        pass

if __name__ == '__main__':

    logParser = LogParser()
    if len(sys.argv) == 2:
        logParser.parselog(sys.argv[1]) # param 1 is log file name
    else:
        logParser.parselog("")
 #---------finish----------------
    print(" ---finished--- ")
    if input("press any key to exit... ... ..."):
        pass