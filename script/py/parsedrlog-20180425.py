import pylab
import re
import sys
import os
import time
import json

import math
import numpy

class LogType:
    MXlog = 0
    GyroScopelog = 1
    LocationLog = 2

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

    def parseJson(strjson):
        #print(strjson)
        json_to_python = json.loads(strjson)
        return json_to_python

    def jsontext(self):
        test = [{"satId": 193, "elevation": 72, "SNRatio": 23, "azimuth": 116},
                {"satId": 17, "elevation": 70, "SNRatio": 27, "azimuth": 71},
                {"satId": 19, "elevation": 68, "SNRatio": 33, "azimuth": 2},
                {"satId": 6, "elevation": 54, "SNRatio": 50, "azimuth": 312},
                {"satId": 9, "elevation": 27, "SNRatio": 33, "azimuth": 122},
                {"satId": 28, "elevation": 26, "SNRatio": 25, "azimuth": 178},
                {"satId": 23, "elevation": 25, "SNRatio": 29, "azimuth": 86},
                {"satId": 2, "elevation": 24, "SNRatio": 20, "azimuth": 279},
                {"satId": 3, "elevation": 22, "SNRatio": 24, "azimuth": 42},
                {"satId": 12, "elevation": 13, "SNRatio": 27, "azimuth": 321}]
        #print(type(test))
        python_to_json = json.dumps(test, ensure_ascii=False)
        #print(python_to_json)
        #print(type(python_to_json))

        json_to_python = json.loads(python_to_json)
        #print(type(json_to_python))


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

    def getlogfiles(path, strfilter, isAll = True):
        try:
            log_files= []
            currentPath = helpFunc.getPathName(path)
            files = os.listdir(currentPath)
            if(not isAll):
                files.sort(key= lambda x : os.stat(currentPath + x).st_mtime)

            for name in files:
                #print("%s %s" %(name, time.ctime(os.stat(currentPath + name).st_mtime)))
                if os.path.isfile(currentPath + name) and re.search(strfilter, name, re.IGNORECASE) is not None:
                    log_files.append( currentPath + name)
            if(not isAll):
                latestFile = log_files[-1]
                log_files.clear()
                log_files.append(latestFile)
                return log_files
            else:
                return log_files
            pass
        except Exception as err:
            print(err)

    def getDirfiles(path, strfilter):
        try:
            log_files= []
            currentPath = helpFunc.getPathName(path)
            for root, dir, files in os.walk(currentPath):
                for name in files:
                    #print("%s %s" %(name, time.ctime(os.stat(currentPath + name).st_mtime)))
                    filename = os.path.join(root, name)
                    result = re.search(strfilter, name, re.IGNORECASE)
                    if os.path.isfile(filename) and re.match(strfilter, name, re.IGNORECASE) is not None:
                        log_files.append(filename)
            return log_files
        except Exception as err:
            print(err)

    def nowtime():
        now = int(time.time())
        return now

    def DateTime_fromString(datetime, strtime):
        #2017/09/28 11:43:05
        firstpart = strtime.split(" ")[0]
        secondpart = strtime.split(" ")[1]
        datetime.m_year = firstpart.split("/")[0]
        datetime.m_month = firstpart.split("/")[1]
        datetime.m_day = firstpart.split("/")[2]

        datetime.m_hours = secondpart.split(":")[0]
        datetime.m_minutes = secondpart.split(":")[1]
        datetime.m_minutes = secondpart.split(":")[2]


class LogParser:

    logType = LogType.GyroScopelog

    def enableParseMxlog(self):
        self.logType = LogType.MXlog

    def enableParseLocationlog(self):
        self.logType = LogType.LocationLog

    def parseLogFiles(self, pathname):
        logfiles = []
        gyroZeroCal = GyroZroTempCalLinReg()
        state = LinReg_state()
        state.temperature_high = 1000
        state.temperature_low = -300
        gyroZeroCal.restoreState(state)
        strfilter = ""
        if (self.logType == LogType.MXlog):
            strfilter = "(^Trace_).*_DLT.txt$"
        elif (self.logType == LogType.GyroScopelog):
            strfilter = "(^gyroscope).*log$"
        elif (self.logType == LogType.LocationLog):
            strfilter = "(^Location-Log-).*log$"

        if(len(pathname) == 0):
            if(self.logType != LogType.LocationLog):
                logfiles = helpFunc.getlogfiles("", strfilter)
            else:
                logfiles = helpFunc.getlogfiles("", strfilter, False)
        elif os.path.isdir(pathname):
            if (self.logType != LogType.LocationLog):
                logfiles = helpFunc.getlogfiles(pathname, strfilter)
            else:
                logfiles = helpFunc.getlogfiles(pathname, strfilter, False)
        elif os.path.isfile(pathname):
            if(self.logType == LogType.MXlog):
                return self.parseMXlog(pathname)
            elif (self.logType == LogType.GyroScopelog):
                return self.parseLog(pathname, gyroZeroCal)
            elif (self.logType == LogType.LocationLog):
                return self.parseLocationLogFiles(pathname)

        for filename in logfiles:
            if (self.logType == LogType.MXlog):
                self.parseMXlog(filename)
            elif (self.logType == LogType.GyroScopelog):
                self.parseLog(filename, gyroZeroCal)
            elif (self.logType == LogType.LocationLog):
                self.parseLocationLogFiles(filename)

    def parseSpeedline(self, line, speedArr):
        ret = False
        items = helpFunc.deal_string(line, ",")
        if (len(items) >= 3):
            speedArr.append(float(items[1]))
            ret = True
        return ret

    def parseMxGyroline(self, line, tempArry, dataArry):
        ret = False
        items = helpFunc.deal_string(line, ",")
        if(len(items) > 4):
            dataArry.append(float(items[1]))
            tempArry.append(float(items[4]))
            ret = True
        return ret

    def parseGyroline(self, line, tempArr, dataArr):
        ret = False
        items = helpFunc.deal_string(line, ",")
        if(len(items) > 7):
            if(float(items[4]) == 0.000000):
                tempArr.append(float(items[3]))
                dataArr.append(float(items[7]))
                ret = True
        return ret

    def pasrseLocationAdasLine(self, line, adasArry):
        ret = False
        items = helpFunc.deal_string(line, ",")
        if(len(items) > 2):
            strAdas= items[2]
            pos = strAdas.find("curAdasValue:")
            if(pos != -1):
                adasArry.append(float(strAdas[pos+13:]))
                ret = True
        return ret

    def pasrseLocationPitchLine(self, line, pitchArry):
        ret = False
        items = helpFunc.deal_string(line, ",")
        if(len(items) > 2):
            strPitch = items[2]
            pos = strPitch.find("pitch:")
            if(pos != -1):
                pitchArry.append(float(strPitch[pos+6:]))
                ret = True
        return ret


    def parseLocationLogFiles(self, logfileName):
        resultList = []
        try:
            adasArry = []
            pitchArry = []

            for line in open(logfileName):
                pos = line.find("[AdasSlope],[Location_Mangner][Dr-Picth]")
                if (pos != -1):
                    if (self.pasrseLocationPitchLine(line[(pos + 40):] , pitchArry)):
                        pass

                pos1 = line.find("[AdasSlope],[Location_Mangner][AdasData]")
                if (pos1 != -1):
                    if (self.pasrseLocationAdasLine(line[pos1 +  40 :], adasArry)):
                        pass

            resultList.append(pitchArry)
            resultList.append(adasArry)
            self.logPlotAdasPich(logfileName, resultList)
            return resultList
        except Exception as err:
            print(err)
    def calMeandata(self, pendingItems, niu, niu2, niu3, sigma):
        if(len(pendingItems)) == 0:
            return

        niu[0] = 0.0
        niu2[0] = 0.0
        niu3[0] = 0.0
        sigma[0] = 0.0
        for data in pendingItems:
            niu[0] += data
            niu2[0] += pow(data,2)
            niu3[0] += pow(data, 3)
        seqCnt = len(pendingItems)
        niu[0] /= seqCnt
        niu2[0] /= seqCnt
        niu3[0] /= seqCnt
        sigma[0] = math.sqrt(niu2[0] - niu[0] * niu[0])

    def skewness(self, niu, niu3 ,sigma):
        try:
            return (niu3 - 3.0 * niu * math.pow(sigma, 2) - math.pow(niu, 3)) / math.pow(sigma, 3)
        except Exception as err:
            print(err)

    def Kurtosis(self, pendingItems, niu, sigma):

        if(len(pendingItems)) == 0:
            return
        try:
            niu4 = 0.0
            for data in pendingItems:
                tmpVal = data
                tmpVal = tmpVal - niu
                niu4 += math.pow(tmpVal, 4.0)

            niu4 /= (float)(len(pendingItems))
            return (niu4 / math.pow(sigma, 4) - 3.0)

        except Exception as err:
            print(err)

    def calStandStillFactor(self, datalist):
        if(len(datalist) > 0):
            average = 0.0
            for item in datalist:
                average += item
            average = average / len(datalist)


            stdevp = 0.0
            for item in datalist:
                stdevp += pow((item - average),2)

            stdevp = stdevp / len(datalist)

            stdevp = math.sqrt(stdevp)

            niu = [0.0]
            niu2 = [0.0]
            niu3 = [0.0]
            sigma = [0.0]
            self.calMeandata(datalist,niu, niu2, niu3, sigma)
            #print("niu:%f, niu2:%f, niu3:%f, sigma:%f" % (niu[0], niu2[0], niu3[0], sigma[0]))
            piandu = self.skewness(niu[0], niu3[0], sigma[0])
            fengdu = self.Kurtosis(datalist, niu[0], sigma[0])

            print("size:%d, average:%d, piandu:%f, fengdu:%f" % (len(datalist), niu[0], piandu, fengdu))

            count = 0
            for item in datalist:
                if(abs(item - average) < stdevp):
                    count += 1
            ratio = (float)(count) / (float)(len(datalist)) * 100.0

            print("ratio:%f, count:%d size:%d stdevp:%f average:%f" % (ratio, count, len(datalist), stdevp, average))


    def parseLog(self, logfileName, gyroZeroCal):
        resultList = []
        try:
            tempArry = []
            dataArry = []
            biasArry=[]
            #clear state
            #state = LinReg_state()
            #state.temperature_high = 1000
            #state.temperature_low = -300
            #gyroZeroCal.restoreState(state)

            #standstill handling
            #isStandStill = False
            #stopCnt = 0
            #standstillDataList= []
            for line in open(logfileName):
                pos = line.find("$Gyroscope")
                if(pos != -1):
                    if(self.parseGyroline(line, tempArry, dataArry)):
                        # standstill handling
                        #if(not isStandStill):
                        #    stopCnt += 1
                        #    if(stopCnt >= 10):
                        #        isStandStill = True
                        #        standstillDataList.append(dataArry[-1])
                        #else:
                        #    if(len(standstillDataList) < 600):
                        #        standstillDataList.append(dataArry[-1])

                        #print("temp:%f, data:%f" % (tempArry[-1], dataArry[-1]))
                        ret = gyroZeroCal.addZroMeasurement(tempArry[-1], dataArry[-1])
                        if (ret != ERetVal.Success):
                            gyroZeroCal.reset()
                            gyroZeroCal.addZroMeasurement(tempArry[-1], dataArry[-1])

                        biasValRefArr = [0.0]

                        gyroZeroCal.estimateZro(tempArry[-1], biasValRefArr)
                        #print("temp:%f, biasVal:%f" %(tempArry[-1],biasValRefArr[0]))
                        #print("a:%f, b:%f" %(gyroZeroCal.geta(), gyroZeroCal.getb()))
                        biasArry.append(biasValRefArr[0])
                    #else: #standstill handling
                    #    if(isStandStill):
                    #        isStandStill = False
                    #        stopCnt = 0
                    #        self.calStandStillFactor(standstillDataList)
                    #        standstillDataList.clear()

            resultList.append(tempArry)
            resultList.append(dataArry)
            resultList.append(biasArry)
            #self.logSavePlot(logfileName, resultList)
            #self.log2yPlots(resultList)
            #self.writelogfile(logfileName, resultList)
            self.logSave2yPlots(logfileName, resultList)
            return resultList
        except Exception as err:
            print(err)


    def parseMXlog(self, logfileName):
        resultList = []
        try:
            tempArry = []
            dataArry = []
            speedArry = []
            biasArry=[]
            startFlg = False
            state = LinReg_state()
            state.temperature_high = 1000
            state.temperature_low = -300
            gyroZeroCal = GyroZroTempCalLinReg()
            gyroZeroCal.restoreState(state)
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
                    if(self.parseMxGyroline(content, tempArry, dataArry)):
                        #print("temp:%f, data:%f" % (tempArry[-1], dataArry[-1]))
                        ret = gyroZeroCal.addZroMeasurement(tempArry[-1], dataArry[-1])
                        if (ret != ERetVal.Success):
                            gyroZeroCal.reset()
                            gyroZeroCal.addZroMeasurement(tempArry[-1], dataArry[-1])

                        biasValRefArr = [0.0]

                        gyroZeroCal.estimateZro(tempArry[-1], biasValRefArr)
                        #print("temp:%f, biasVal:%f" %(tempArry[-1],biasValRefArr[0]))
                        #print("a:%f, b:%f" %(gyroZeroCal.geta(), gyroZeroCal.getb()))
                        biasArry.append(biasValRefArr[0])

                        startFlg = False
            resultList.append(tempArry)
            resultList.append(dataArry)
            resultList.append(biasArry)
            #self.logSavePlot(logfileName, resultList)
            #self.log2yPlots(resultList)
            #self.writelogfile(logfileName, resultList)
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

    def logPlotAdasPich(self, filename, resultList):

        pylab.figure()
        pylab.plot(resultList[0], label='pitch')
        pylab.plot(resultList[1], label='adas')

        pylab.legend()

        pylab.show()
        pylab.close()

    def logSave2yPlots(self, filename, resultList):
        basename = os.path.basename(filename)
        index = basename.rfind(".")
        titlename = basename[:index]
        filepath = os.path.dirname(filename)
        pic_path = filepath + os.path.sep + "pic" + os.path.sep
        pic_filename = pic_path + titlename + ".png"

        fig = pylab.figure()
        ax1 = fig.add_subplot(111)

        ax1.plot(resultList[1], label = 'gyroData')
        ax1.set_ylabel("gyroData")
        ax1.legend(loc=9)

        ax1.plot(resultList[2], label = 'zerogyro')
        ax1.legend(loc=9)

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
################################################
class Point:
    def __init__(self):
        self.x = 0
        self.y = 0

class DateTime:
    def __init__(self):
        self.m_hours = 0
        self.m_minutes = 0
        self.m_seconds = 0
        self.m_year = 0
        self.m_month = 0
        self.m_day = 0

class Satellite:
    def __init__(self):
        self.satId = 0; #satellite
        self.elevation = 0; # [0, 90]
        self.azimuth = 0; # [0, 359]
        self.SNRatio = 0; # [0, 99]

class GpsInfo:
    def __init__(self):
        self.m_isValid = False;
        self.m_dateTime = DateTime();
        self.m_timestamp = 0
        self.m_Pos = Point()
        self.m_altitude = 0
        self.m_ori = 0;
        self.m_speed = 0;
        self.m_latString = ""
        self.m_lonString = ""
        self.m_quality3D = False
        self.m_hdop = 0
        self.m_pdop = 0
        self.m_vdop = 0
        self.m_satInViewNum = 0
        self.m_satNum = 0
        self.m_satellites = []
        self.m_bdSatNum = 0
        self.m_bdSatellites = []
        self.m_stamp = 0
        self.m_angle = 0

class SGyroSensorData:
    def __init__(self):
        self.seq_nr = 0
        self.dirSwitch = False
        self.deltaTime = 0
        self.gyroTempe = 0
        self.accelTempe = 0
        self.canSpeed = 0.0
        self.gyroOutX = 0.0
        self.gyroOutY = 0.0
        self.gyroOutZ = 0.0
        self.accelOutX = 0.0
        self.accelOutY = 0.0
        self.accelOutZ = 0.0

class SensorAccuracy:
    def __init__(self):
        self.SensorAccuracy_unreliable = 0
        self.SensorAccuracy_low = 1
        self.SensorAccuracy_medium = 2
        self.SensorAccuracy_high = 3

class AccelerometerData:
    def __init__(self):
        self.accuracy = SensorAccuracy()
        self.x = 0.0
        self.y = 0.0
        self.z = 0.0

class DrLogDataCheck:

    def checkGyroscopeFiles(self, pathname):

        strfilter = "(^gyroscope).*log$"
        logfiles = []

        if (len(pathname) == 0):
            logfiles = helpFunc.getDirfiles("", strfilter)
        elif os.path.isdir(pathname):
            logfiles = helpFunc.getDirfiles(pathname, strfilter)
        elif os.path.isfile(pathname):
            self.checkGyroscopeFile(filename)

        for filename in logfiles:
            self.checkGyroscopeFile(filename)

    def checkGyroscopeFile(self, filename):
        resultList = []
        try:
            for line in open(filename):
                pos = line.find("$GPS")
                if (pos != -1):
                    gpsinfo = self.parseGps(line)
                    snr = self.calSnrAvg(gpsinfo)
                    if(snr > 0):
                        resultList.append(snr)
        except Exception as err:
            print(err)

    def calSnrAvg(self, gpsinfo):
        SNRatio_num = 0
        SNRatio_avg = 0
        for i, satelite in enumerate(gpsinfo.m_satellites):
            if(satelite.SNRatio > 30):
                SNRatio_num += 1
            SNRatio_avg += satelite.SNRatio

        if(len(gpsinfo.m_satellites) > 0):
            SNRatio_avg = SNRatio_avg / len(gpsinfo.m_satellites)
        return SNRatio_avg


    def logPlot(self, resultList):
        pylab.figure()
        pylab.plot(resultList,  label = 'snr')
        pylab.legend()

        pylab.show()
        pass

    def parseGps(self, line):
        gpsInfo = GpsInfo()
        gpsInfo.m_timestamp = int(line.split("-")[0])
        pos = line.find("$GPS,") + 5
        context = line[pos:]
        #print(context)
        items = helpFunc.deal_string(context, ",")
        for i, val in enumerate(items):
            if(i ==0):
                gpsInfo.m_isValid = items[i]
            elif(i == 1):
                helpFunc.DateTime_fromString(gpsInfo.m_dateTime, str(items[i]))
            elif(i == 2):
                gpsInfo.m_Pos.x = int(items[i])
            elif(i == 3):
                gpsInfo.m_Pos.y = int(items[i])
            elif(i == 4): #altitude
                gpsInfo.m_altitude = int(items[i])
            elif(i == 5):
                gpsInfo.m_ori = int(items[i])
            elif(i == 6):
                gpsInfo.m_speed = int(items[i])
            elif(i == 7):
                gpsInfo.m_quality3D = int(items[i])
            elif(i == 8):
                gpsInfo.m_hdop = int(items[i])
            elif(i == 9):
                gpsInfo.m_pdop = int(items[i])
            elif(i == 10):
                gpsInfo.m_vdop = int(items[i])
            elif(i == 11):
                gpsInfo.m_satInViewNum = int(items[i])
            elif(i == 12):
                gpsInfo.m_satNum = int(items[i])
            elif(i == 13):
                gpsInfo.m_stamp = int(items[i])
            elif(i == 14):
                gpsInfo.m_angle = int(items[i])
            elif(i == 15): #json
                jsonlist = helpFunc.parseJson(",".join(items[i:]))
                #print(jsonlist)
                for j, jsonitem in enumerate(jsonlist):
                    satellite = Satellite()
                    gpsInfo.m_satellites.append(satellite)
                    gpsInfo.m_satellites[j].satId = jsonlist[j].get('satId')
                    gpsInfo.m_satellites[j].elevation = jsonlist[j].get('elevation')
                    gpsInfo.m_satellites[j].azimuth = jsonlist[j].get('azimuth')
                    gpsInfo.m_satellites[j].SNRatio = jsonlist[j].get('SNRatio')

        return gpsInfo

    def parseGyro(self, line, gyroInfo):
        helpfuncOp = helpFunc()
        pos = line.find("$Gyroscope,") + 11
        context = line[pos:]
        items = helpfunc.deal_string(context, ",")

        for i, val in enumerate(items):
            if (i == 0):
                gyroInfo.dirSwitch = int(items[i])
            elif (i == 1):
                gyroInfo.deltaTime = int(items[i])
            elif (i == 2):
                gyroInfo.temperature = int(items[i])
            elif (i == 3):
                gyroInfo.speed = float(items[i])
            elif (i == 4):  # altitude
                gyroInfo.gyroOutX = float(items[i])
            elif (i == 5):
                gyroInfo.gyroOutY = float(items[i])
            elif (i == 6):
                gyroInfo.gyroOutZ = float(items[i])

    def parseAccel(self, line, accInfo):
        helpfuncOp = helpFunc()
        pos = line.find("$Accel,") + 7
        context = line[pos:]
        items = helpfunc.deal_string(context, ",")

        for i, val in enumerate(items):
            if (i == 0):
                accInfo.accuracy = int(items[i])
            elif (i == 1):
                accInfo.timeStamp = int(items[i])
            elif (i == 2):
                accInfo.x = float(items[i])
            elif (i == 3):
                accInfo.y = float(items[i])
            elif (i == 4):  # altitude
                accInfo.z = float(items[i])


if __name__ == '__main__':

    logParser = LogParser()
    #logParser.enableParseMxlog()
    #logParser.enableParseLocationlog()
    drlogcheck = DrLogDataCheck()
    if len(sys.argv) == 2:
        #logParser.parseLogFiles(sys.argv[1])  # param 1 is log file name
        drlogcheck.checkGyroscopeFiles(sys.argv[1])
    else:
        #logParser.parseLogFiles("")
        drlogcheck.checkGyroscopeFiles("")

 #---------finish----------------
    print(" ---finished--- ")
    if input("press any key to exit... ... ..."):
        pass