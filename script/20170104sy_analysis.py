#!/usr/bin/env python
#-*- coding: utf-8 -*-


__author__ = ['"anjy" <anjy@mapbar.com>', '"zhougang" <zhougang@mapbar.com>']


"""
Sample output:

analysis.py memory.txt NaviCore
map-render-nc          800 B     0.78 K   0.00 M
map-render         1151780 B  1124.79 K   1.10 M
guidance               160 B     0.16 K   0.00 M
junction-view         1624 B     1.59 K   0.00 M
render-system     12255276 B 11968.04 K  11.69 M
cq_stdlib          2558050 B  2498.10 K   2.44 M
other             22946580 B 22408.77 K  21.88 M
routing-nc         9986040 B  9751.99 K   9.52 M
location              4124 B     4.03 K   0.00 M
graphics           1358216 B  1326.38 K   1.30 M
tmc-reporter            64 B     0.06 K   0.00 M
map2d                  312 B     0.30 K   0.00 M
real3d                3208 B     3.13 K   0.00 M
ALL               50266234 B 49088.12 K  47.94 M

"""


import os
import sys
import glob
import csv


FILE_NAME = os.path.basename(__file__)
HELP_STR = """
Usage:

\t%s <filename> <Jenkins Project Name>

Examples:

\t%s memory.txt NaviCoreFlavor
\t%s memory.txt NaviCoreLinuxBranch

Sample Output:

\tmap-render-nc          800 B     0.78 K   0.00 M
\tmap-render         1151780 B  1124.79 K   1.10 M
\tguidance               160 B     0.16 K   0.00 M
\tjunction-view         1624 B     1.59 K   0.00 M
\trender-system     12255276 B 11968.04 K  11.69 M
\tcq_stdlib          2558050 B  2498.10 K   2.44 M
\tother             22946580 B 22408.77 K  21.88 M
\trouting-nc         9986040 B  9751.99 K   9.52 M
\tlocation              4124 B     4.03 K   0.00 M
\tgraphics           1358216 B  1326.38 K   1.30 M
\ttmc-reporter            64 B     0.06 K   0.00 M
\tmap2d                  312 B     0.30 K   0.00 M
\treal3d                3208 B     3.13 K   0.00 M
\tALL               50266234 B 49088.12 K  47.94 M
""" % (FILE_NAME, FILE_NAME, FILE_NAME)


########################################################################################
#get all mememory text files
########################################################################################
def getAllMemFiles(path):
    try:
        file_list= []
        for file_name in glob.glob(getPathName(path) + r"*memory*.txt"):
            file_list.append(file_name)
        return file_list
    except Exception as err:
        print(err)

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

def parseLine(line):
    splitString=["NaviCoreBuildAllWindows", "navicorebuildallwindows", "navicore_dl", "BusinessSink"]
    for item in splitString:
        pieces = line.replace("\\", "/").split(item + "/")
        if len(pieces) > 1:
            break;
    return pieces;

def handleFile(sp, filename):
    modules = dict()
    for line in open(filename):
        pieces = line.replace("\\", "/").split(sp + "/")
        if len(pieces) == 1:  # other
            pieces = parseLine(line)
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

    all_memory = 0
    for k, v in modules.items():
       # print("{:15} {:10} B {:8.2f} K {:6.2f} M".format(k, v, v / 1024.0, v / 1024.0 / 1024.0))
        all_memory += v
    #print("{:15} {:10} B {:8.2f} K {:6.2f} M".format("ALL", all_memory, all_memory / 1024.0,
    #                                                 all_memory / 1024.0 / 1024.0))
    modules["file_all_memory"] = "{:15} {:10} B {:8.2f} K {:6.2f} M".format("ALL", all_memory, all_memory / 1024.0,
                                                     all_memory / 1024.0 / 1024.0)
    modules["file_name"] = filename
    return modules

def writeCsvFile(fn, results) :
    if len(results) == 0:
        return
    outputfile = getPathName(fn) + "result.csv"
    filename = ""
    try:
        with open(outputfile, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['no', "time", "mem used details", "all mem used"])
            time = memdetails = allmem = ""
            i = 0
            for modules in results:
                i = i + 1
                filename = modules["file_name"]
                time =  filename[:-11]
                time = 'time' + ''.join(time.split('\\')[-1])
                allmem = modules["file_all_memory"]
                for k, v in modules.items():
                    if k != "file_name" and k != "file_all_memory":
                        memdetails +=  str("{:15} {:10} B {:8.2f} K {:6.2f} M".format(k, v, v / 1024.0, v / 1024.0 / 1024.0)) + "\n"
                writer.writerow([i, time, memdetails, allmem])
                time = memdetails = allmem = ""
            csvfile.close()
    except Exception as err:
     print("file %s write results failed:[%s]" % (filename, err))
pass


def analyze(sp, fn): #sp is module filter, fn is
    if len(sp) == 0:
        sp = "src"
    filelist = getAllMemFiles(fn)
    results = []
    if filelist is not None:
        for filename in filelist:
            results.append(handleFile(sp, filename))
    writeCsvFile(fn, results)

if __name__ == '__main__':
    if len(sys.argv) == 3:
        analyze(sys.argv[2], os.path.abspath(sys.argv[1])) #param 2 is filter name, param 1 is folder name
    elif len(sys.argv) == 2:
        analyze(sys.argv[1], "")  # param 1 is filter name, param 2 is folder name
    else:
        analyze("src", "")
        #sys.exit(HELP_STR)
    if input("finished, press any key to exit... ... ..."):
        pass
