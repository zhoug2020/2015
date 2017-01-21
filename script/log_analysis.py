#!/usr/bin/env python
#-*- coding: utf-8 -*-


__author__ = ['"zhougang" <zhougang@mapbar.com>']

import os
import sys
import glob
import csv


FILE_NAME = os.path.basename(__file__)
'''
Usage:
\t%s <filename> <Jenkins Project Name>
'''
VewMap = {
    1 : "MapForm",
    2 : "MapForm_set",
}

class LogData():
    def __init__(self):
        #check datae
        self.state = 0
        self.preViewId = -1
        self.viewId = -1
        self.totalTime = 0
        #self.keys = ""
        self.viewData = []
        self.controlData = []
########################################################################################
#get all mememory text files
########################################################################################
def getPathName(path):
    if len(path) == 0:
        currentPath = os.getcwd()
    else:
        currentPath = path
    if currentPath[:-1] != os.path.sep:
        currentPath += os.path.sep
    return currentPath

def addMemory(key, val, modules):
    if key not in modules:
        modules[key] = 0
    modules[key] += int(val[-1].split(':')[-1])

def getId(line):
    id = -1
    pos = line.find("id=")
    if (pos != -1):
        id = line[pos + 3:].split()[0]
    return id

def parseLine(line):
    # get id
    modules = dict()
    modules["ViewId"] = getId(line)
    modules["Action"] = "".join(line.split(",")[2:-1]).strip()
    modules["Time"] = int(line.split(":")[-1])
    modules["ItemType"] = "".join("".join(line.split(",")[0]).split()[-1])
    return modules

def addViewData(list, modules):
    if len(list) > 0 and list[-1]["Action"] == "VIEW_TRANS_CALLAGAIN" and modules["Action"] == "VIEW_TRANS_CHANGE":
        list[-1]["Action"] = "VIEW_TRANS_CHANGE"
    else:
        list.append(modules)

def getCurrentData(list, curId):
    for v in reversed(list):
        if v.state >=1 and v.state <=5 and v.viewId == curId:
            return v

def getCurrentDataIgnoreState(list, curId):
    for v in reversed(list):
        if v.viewId == curId:
            return v

def createLogData(result, curId, preId):
    print(curId)
    data = LogData()
    data.viewId = curId
    data.preViewId = preId
    data.state = 1
    #data.keys =  str(curId) + "-" + str(preId)
    result.append(data)
    preId = curId
    return data

def setDataState(data, state):
    if (state == "CLIENT_ONCREATE"):
        data.state = 1
    elif (state == "VIEW_ONLAYOUT"):
        data.state = 2
    elif (state == "CLIENT_ONFOREGROUND"):
        data.state = 3
    elif (state == "CLIENT_ONPAIT"):
        data.state = 4
    elif (state == "CLIENT_ONBACKGROUND"):
        data.state = 5
    elif (state == "CLIENT_ONDESTROY"):
        data.state = 6


def handleFile(filename):
    result = []
    preId = curId = -1
    curViewId = -1
    curkey = prekey = ""
    for line in open(filename):
        modules = parseLine(line)
        curId = modules["ViewId"]
        if (preId != curId) and line.find("ITEM_VIEW") != -1 and modules["Action"] == "CLIENT_ONCREATE":  #transform view
            data = createLogData(result, curId, preId)
            addViewData(data.viewData, modules)
            setDataState(data, modules["Action"])
            preId = curId
            curViewId = curId
        elif line.find("ITEM_VIEW") != -1 or line.find("ITEM_TRANS") != -1:
            if (line.find("ITEM_VIEW") != -1 and curViewId == 1):
                curViewId = curId
            data = getCurrentData(result, curId)
            '''
            if data is None:
                print("dddddddddddddd")
            if data.logdata is None:
                print("dddddddddddddd")
           '''
            if data is not None:
                pass
                #print("{0}:{1}".format(data.viewId, data.state))
            if data is None:
                data = createLogData(result, curId, preId)
                addViewData(data.viewData, modules)
                preId = curId
                curViewId = curId
            #elif data.state == 5 and modules["Action"] == "CLIENT_ONFOREGROUND":
            elif modules["Action"] == "VIEW_ONLAYOUT":
                if data.state == 5:
                    data = createLogData(result, curId, preId)
                    addViewData(data.viewData, modules)
                    preId = curId
                    curViewId = curId
                else:
                    pass
                    print("{0}:{1}".format(data.viewId, data.state))
            else:
                addViewData(data.viewData,modules)
                setDataState(data, modules["Action"])
        elif line.find("ITEM_CONTROL") != 1:
            data = getCurrentDataIgnoreState(result, curViewId)
            if data is not None:
                data.controlData.append(modules)
            else:
                print("invalid control")

        '''
        if len(pieces) == 1:  # other
            pieces = p  arseLine(line)
            if len(pieces) == 1: #other
                n = "other"
                #print("other1 is %s" % pieces)
            else:
                n = pieces[1].split('/')[0]
            addMemory(n, pieces, modules)
        else:
            if len(pieces[0]) > 0:
                n = pieces[0].split('/')[-2]
                addMemory(n, pieces, modules)
            else:
                #n = pieces[1].split('.')[0]
                n = "Client"
                if len(n) == 0:
                    n = "other"
                    #print("other2 is %s" % pieces)
                addMemory(n, pieces, modules)
        '''
    for data in result:
       # print("{:15} {:10} B {:8.2f} K {:6.2f} M".format(k, v, v / 1024.0, v / 1024.0 / 1024.0))
       addClinetOnPaint = False;
       addViewOnPaint = False;
       all_time = 0
       for module in data.viewData:
           action = module["Action"]
           if not addClinetOnPaint and  action == "CLIENT_ONPAIT":
               all_time += int(module["Time"])
               addClinetOnPaint = True
           if not addViewOnPaint and action == "VIEW_ONPAIT":
               all_time += int(module["Time"])
               addViewOnPaint = True
           elif action != "CLIENT_ONPAIT" or action != "VIEW_ONPAIT":
               all_time += int(module["Time"])
       data.totalTime = all_time
       #print("%d,%d", (data.viewId, all_time))
    return result

def writeCsvFile(results) :
    if len(results) == 0:
        return
    outputfile = getPathName("") + "result.csv"
    filename = ""
    try:
        with open(outputfile, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['no', "preView", "currentView", "usedTime"])
            preView  = currentView = usedTime= ""
            i = 0
            for data in results:
                i = i + 1
                preView = data.preViewId
                currentView = data.viewId
                usedTime = data.totalTime
                writer.writerow([i, preView, currentView, usedTime])
                preView = currentView = usedTime = ""
            writer.writerow([])
            writer.writerow([])
            writer.writerow(["time used details:"])

            # write view data
            for data in results:
                i = 0
                preView = data.preViewId
                currentView = data.viewId
                usedTime = data.totalTime
                writer.writerow(["{} -> {}".format(preView,currentView)])
                writer.writerow(['no', "itemType", "action", "usedTime"])
                for module in data.viewData:
                    i +=1
                    itemType = action = time = ""
                    itemType = module["ItemType"]
                    action = module["Action"]
                    time = module["Time"]
                    writer.writerow([i, itemType, action, time])
                    itemType = action = time = ""
            #write control data
            for data in results:
                i = 0
                writer.writerow(["viewId : {}".format(currentView)])
                writer.writerow(['no', "itemType", "Id", "ViewId", "action", "usedTime"])
                for module in data.controlData:
                    i +=1
                    itemid = itemType = action = time = ""
                    itemid = module["ViewId"]
                    itemType = module["ItemType"]
                    action = module["Action"]
                    time = module["Time"]
                    writer.writerow([i, itemType, itemid, data.viewId, action, time])
                    itemid = itemType = action = time = ""
            csvfile.close()
    except Exception as err:
     print("file %s write results failed:[%s]" % (filename, err))
pass


def analyze(logfile): #sp is module filter, fn is
    if len(logfile) == 0 :
        logfile = getPathName(logfile) + "timeused_log.txt"
    result = handleFile(logfile)
    writeCsvFile(result)

if __name__ == '__main__':
    if len(sys.argv) == 2:
        analyze(sys.argv[1])  # param 1 is log file name
    else:
        analyze("")
        #sys.exit(HELP_STR)
    if input("finished, press any key to exit... ... ..."):
        pass
