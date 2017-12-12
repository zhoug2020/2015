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

class LogItem :
    lon = 0.0
    lat = 0.0
    heading = 0.0
    turnRate = 0.0
    speed = 0.0

class KalmanScaleFactorItem:
    gyroBias = 0.0
    gyroCorrection = 0.0
    gyroScaleFactor = 0.0
    wheelScaleFactor = 0.0

class DrDiagInfo:
    def __init__(self):
        self.kalmanlogItem = LogItem()
        self.gpslogItem = LogItem()
        self.rawLogItem = LogItem()
        self.kalmanSCItem = KalmanScaleFactorItem()

class ResultItemList:
    drItemList = []
    gpsItemList = []
    rawDrItemList= []
    kalmanItemList= []
    kalmanScaleFactorList = []


    def clear(self):
        self.drItemList.clear()
        self.gpsItemList.clear()
        self.rawDrItemList.clear()
        self.kalmanItemList.clear()

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
                if os.path.isfile(currentPath + name) and re.search("(^dr-log).*log$", name, re.IGNORECASE) is not None:
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

    def parseDr(self, line):
        drLogItem = LogItem()
        pos = line.find("[_dumpDrInfo]") + 13
        context = line[pos:]
        items = helpFunc.deal_string(context, ",")

        for i, val in enumerate(items):

            if (i == 2):
                #print(items[i])
                lat = helpFunc.deal_string(items[i], ":")
                drLogItem.lat = float(lat[1])
                #print(drLogItem.lat)
            elif (i == 3):
                #print(items[i])
                lon = helpFunc.deal_string(items[i], ":")
                drLogItem.lon = float(lon[1])
                #print(drLogItem.lon)
            elif (i == 4):
                #print(items[i])
                speed = helpFunc.deal_string(items[i], ":")
                drLogItem.speed = float(speed[1])
                #print(drLogItem.speed)
            elif (i == 5):
                #print(items[i])
                heading = helpFunc.deal_string(items[i], ":")
                drLogItem.heading = float(heading[1])
                #print(drLogItem.heading)
        return drLogItem

    def parseDrDiag(self, line):
        drDiagInfo = DrDiagInfo()
        pos = line.find("[_DrDiagInfo]") + 13
        context = line[pos:]
        items = helpFunc.deal_string(context, ",")

        if(len(items) > 10):
            for i, val in enumerate(items):
                if (i == 0):
                    drDiagInfo.rawLogItem.lon = float(items[i])
                elif (i == 1):
                    drDiagInfo.rawLogItem.lat = float(items[i])
                elif (i == 2):
                    drDiagInfo.rawLogItem.heading = float(items[i])
                elif (i == 3):
                    drDiagInfo.kalmanlogItem.lon = float(items[i])
                elif (i == 4):
                    drDiagInfo.kalmanlogItem.lat = float(items[i])
                elif (i == 5):
                    drDiagInfo.kalmanlogItem.heading = float(items[i])
                elif (i == 6):
                    drDiagInfo.gpslogItem.lon = float(items[i])
                elif (i == 7):
                    drDiagInfo.gpslogItem.lat = float(items[i])
                elif (i == 8):
                    drDiagInfo.gpslogItem.heading = float(items[i])
                elif (i == 9):
                    drDiagInfo.kalmanSCItem.gyroBias= float(items[i])
                elif (i == 10):
                    drDiagInfo.kalmanSCItem.gyroCorrection = float(items[i])
                elif (i == 11):
                    drDiagInfo.kalmanSCItem.gyroScaleFactor = float(items[i])
                elif (i == 12):
                    drDiagInfo.kalmanSCItem.wheelScaleFactor = float(items[i])
        else:
            for i, val in enumerate(items):
                if (i == 0):
                    drDiagInfo.rawLogItem.lon = float(items[i])
                elif (i == 1):
                    drDiagInfo.rawLogItem.lat = float(items[i])
                elif (i == 2):
                    drDiagInfo.rawLogItem.heading = float(items[i])
                elif (i == 3):
                    drDiagInfo.kalmanlogItem.lon = float(items[i])
                elif (i == 4):
                    drDiagInfo.kalmanlogItem.lat = float(items[i])
                elif (i == 5):
                    drDiagInfo.kalmanlogItem.heading = float(items[i])
                elif (i == 6):
                    drDiagInfo.gpslogItem.lon = float(items[i])
                elif (i == 7):
                    drDiagInfo.gpslogItem.lat = float(items[i])
                elif (i == 8):
                    drDiagInfo.gpslogItem.heading = float(items[i])
        return drDiagInfo

    def parselog(self, logfileName):

        if(len(logfileName) == 0): #empty
            logfileName = helpFunc.getlogfiles("")
        elif os.path.isdir(logfileName):
            logfileName = helpFunc.getlogfiles(logfileName)
            print(logfileName)
        resultList = ResultItemList()
        try:
            for line in open(logfileName):
                if(line.find("[_dumpDrInfo]") != -1):
                    drItem = self.parseDr(line)
                    resultList.drItemList.append(drItem)
                elif(line.find("_DrDiagInfo") != -1):
                    result =  self.parseDrDiag(line)
                    resultList.rawDrItemList.append(result.rawLogItem)
                    resultList.kalmanItemList.append(result.kalmanlogItem)
                    resultList.gpsItemList.append(result.gpslogItem)
                    resultList.kalmanScaleFactorList.append(result.kalmanSCItem)

            self.logPlot(resultList)

            resultList.clear()

        except Exception as err:
            print(err)


    def logPlot(self, resultList):
        drHeadingValArr = []
        gpsHeadingValArr = []
        rawDrHeadingValArr = []

        for item in resultList.drItemList:
            drHeadingValArr.append(item.heading)

        for item in resultList.gpsItemList:
            gpsHeadingValArr.append(item.heading)

        for item in resultList.rawDrItemList:
            rawDrHeadingValArr.append(item.heading)

        pylab.figure()
        pylab.plot(drHeadingValArr,  label = 'DrHeading')
        pylab.plot(gpsHeadingValArr, label= 'GpsHeading')
        pylab.plot(rawDrHeadingValArr, label='rawDrHeading')
        pylab.legend()

        gyroBiasArr = []
        gyroCorrectionArr = []
        gyroScaleFactorArr = []
        wheelScaleFactorArr = []
        for item in resultList.kalmanScaleFactorList:
            gyroBiasArr.append(item.gyroBias)
            gyroCorrectionArr.append(item.gyroCorrection)
            gyroScaleFactorArr.append(item.gyroScaleFactor)
            wheelScaleFactorArr.append(item.wheelScaleFactor)

        pylab.figure()
        pylab.plot(gyroBiasArr,  label = 'gyroBias')
        pylab.plot(gyroCorrectionArr, label= 'gyroCorrection')
        pylab.plot(gyroScaleFactorArr, label='gyroScaleFactor')
        pylab.plot(wheelScaleFactorArr, label='wheelScaleFactor')
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