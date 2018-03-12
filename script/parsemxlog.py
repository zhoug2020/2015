import numpy
import pylab
import re
import sys
import os
import time

import math
from matplotlib.ticker import MultipleLocator, FuncFormatter


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

    def getlogfile(path):
        log_files =  helpFunc.getlogfiles(path)
        return log_files[-1]

    def getlogfiles(path):
        try:
            log_files= []
            currentPath = helpFunc.getPathName(path)
            files = os.listdir(currentPath)
            #files.sort(key= lambda x : os.stat(currentPath + x).st_mtime)
            for name in files:
                #print("%s %s" %(name, time.ctime(os.stat(currentPath + name).st_mtime)))
                if os.path.isfile(currentPath + name) and re.search("(^Trace_)*_DLT.txt$", name, re.IGNORECASE) is not None:
                    log_files.append( currentPath + name)
            return log_files
            pass
        except Exception as err:
            print(err)

    def nowtime():
        now = int(time.time())
        return now

class LogParser:

    def parseLogFiles(self, pathname):
        logfiles = []
        if(len(pathname) == 0):
            logfiles = helpFunc.getlogfiles("")
        elif os.path.isdir(pathname):
            logfiles = helpFunc.getlogfiles(pathname)

        for filename in logfiles:
            self.parselog(filename)

    def parseSpeedline(self, line, speedArr):
        items = helpFunc.deal_string(line, ",")
        if (len(items) >= 3):
            speedArr.append(float(items[1]))

    def parseGyroline(self, line, tempArry, dataArry):
        items = helpFunc.deal_string(line, ",")
        if(len(items) > 4):
            dataArry.append(float(items[1]))
            tempArry.append(float(items[4]))


    def parselog(self, logfileName):
        resultList = []
        try:
            tempArry = []
            dataArry = []
            speedArry = []
            startFlg = False
            for line in open(logfileName):
                pos0 = line.find("VehicleSpeedData")
                pos1 = line.find("GyroscopeData")
                if(pos0 != -1):
                    content= line[pos0:]
                    self.parseSpeedline(content, speedArry)
                    if (not startFlg and speedArry[-1] == 0.000000):
                        startFlg = True
                    elif (startFlg and speedArry[-1] > 0.000000):
                        startFlg = False
                elif (pos1 != -1 and startFlg):
                    content = line[pos1:]
                    self.parseGyroline(content, tempArry, dataArry)
                    startFlg = False
            resultList.append(tempArry)
            resultList.append(dataArry)
            #self.logSavePlot(logfileName, resultList)
            #self.log2yPlots(resultList)
            self.writelogfile(logfileName, resultList)
            self.logSave2yPlots(logfileName, resultList)
            return resultList
        except Exception as err:
            print(err)

    def writelogfile(self, filename, resultList):
        basename = os.path.basename(filename)
        index = basename.rfind(".")
        titlename = basename[:index]
        filepath = os.path.dirname(filename)
        pic_path = filepath + os.path.sep + "zero" + os.path.sep
        zero_filename = pic_path + titlename + ".txt"

        with open(zero_filename, 'w') as f:
            for i  in range(len(resultList[0])):
                linedata = "{}\t{}\n".format(resultList[0][i],  resultList[1][i])
                f.writelines(linedata)

    def logSavePlot(self, filename, resultList):
        basename = os.path.basename(filename)
        index = basename.rfind(".")
        titlename = basename[:index]
        filepath = os.path.dirname(filename)
        pic_path = filepath + os.path.sep + "pic" + os.path.sep
        pic_filename = pic_path + titlename + ".png"

        pylab.figure()
        pylab.plot(resultList[1],  label = 'zeroGyro')
        pylab.legend()

        pylab.savefig(pic_filename)

    def log2yPlots(self, resultList):
        fig = pylab.figure()
        ax1 = fig.add_subplot(111)

        ax1.plot(resultList[1], label = 'zeroGyro')
        ax1.set_ylabel("zeroGyro")
        ax1.legend(loc=2)
        ax2 = ax1.twinx() #add y axis
        ax2.plot(resultList[0], '-r', label = 'tempture')
        ax2.set_ylim([-30, 30])
        ax2.set_ylabel("tempture")
        ax2.legend(loc=1)

        pylab.show()
        pylab.close()
        pass

    def logSave2yPlots(self, filename, resultList):
        basename = os.path.basename(filename)
        index = basename.rfind(".")
        titlename = basename[:index]
        filepath = os.path.dirname(filename)
        pic_path = filepath + os.path.sep + "pic" + os.path.sep
        pic_filename = pic_path + titlename + ".png"

        fig = pylab.figure()
        ax1 = fig.add_subplot(111)

        ax1.plot(resultList[1], label = 'zeroGyro')
        ax1.set_ylabel("zeroGyro")
        ax1.legend(loc=2)
        ax2 = ax1.twinx() #add y axis
        ax2.plot(resultList[0], '-r', label = 'tempture')
        ax2.set_ylim([-30, 30])
        ax2.set_ylabel("tempture")
        ax2.legend(loc=1)

        pylab.savefig(pic_filename)
        pylab.close()

    def logPlot(self, resultList):

        pylab.figure()
        pylab.plot(resultList[1],  label = 'zeroGyro')
        pylab.legend()

        pylab.show()
        pass

if __name__ == '__main__':

    logParser = LogParser()
    if len(sys.argv) == 2:
        logParser.parseLogFiles(sys.argv[1]) # param 1 is log file name
    else:
        logParser.parseLogFiles("")
 #---------finish----------------
    print(" ---finished--- ")
    if input("press any key to exit... ... ..."):
        pass