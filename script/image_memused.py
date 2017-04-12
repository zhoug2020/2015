import os
import sys
import re
import math
from PIL import Image
import csv

class ImageInfo:
    def __init__(self):
        self.name = ""
        self.filepath = ""
        self.orgin_width = 0
        self.orgin_height = 0
        self.width = 0
        self.height = 0
        self.size = 0
        self.id = 0

class ViewData():
    def __init__(self):
        #check datae
        self.filepath = ""
        self.name = ""
        self.memsize = 0
        self.details = []

def getImagePath():
    currentPath = os.getcwd()
    rpos = currentPath.rfind(os.path.sep)
    if rpos != -1 and currentPath[0:rpos].find("images") != -1:
       # print(currentPath[0:rpos])
        return currentPath[0:rpos] + os.path.sep
    else:
        return ""

def getPathName(path):
    if len(path) == 0:
        currentPath = os.getcwd()
    else:
        currentPath = path
    if currentPath[:-1] != os.path.sep:
        currentPath += os.path.sep
    return currentPath

def getAllUifiles(path):
    try:
        ui_files= {}
        currentPath = getPathName(path)
        files = os.listdir(currentPath)
        for name in files:
            if os.path.isfile(name) and re.search(".txt$", name, re.IGNORECASE):
                ui_files[name] = currentPath + name
        return ui_files
        pass
    except Exception as err:
        print(err)

def calculateView(viewfile, viewdata):

    if len(viewfile) == 0:
        return

    try:
        #print(viewfile)
        fp = open(viewfile, "r",encoding="utf-16")
        lines = fp.readlines()
        fp.close()
        for line in lines:
            if line.find("#include ") != -1:
                libary = line.split('\"')[1]
                calculateView(libary, viewdata)
            elif line.find(".png") != -1:
                pngname = line.split('=')[-1].strip()
                pngname = pngname.replace("/", os.path.sep)
                img_path = getImagePath() + pngname
                img = Image.open(img_path, "r")
                info = ImageInfo()
                (width, height) = img.size
                info.orgin_width = width
                info.orgin_height = height
                info.name = pngname
                wpower = math.ceil(math.log2(width))
                hpower = math.ceil(math.log2(height))
                info.width = math.pow(2,wpower)
                info.height = math.pow(2,hpower)
                info.size = info.width * info.height * 4
                #print("%s, origin widh:%d, origin height:%d, width:%d,height%d,size:%d" %(info.name, info.orgin_width, info.orgin_height, info.width, info.height, info.size))
                viewdata.details.append(info)
        return True
    except Exception as err:
        print("file %s operation failed:[%s], try to use another encoding!" % (viewfile, err))
        return False

def analysis(uipath):
    allUifiles = getAllUifiles(uipath)
    results = []
    print("len is %d\n" % (len(allUifiles)))
    for name, filename in allUifiles.items():
        view_data = ViewData()
        view_data.name = name
        view_data.filepath = filename
        calculateView(filename, view_data)
        all_memory = 0
        for picinfo in view_data.details:
            all_memory = all_memory + picinfo.size
        view_data.memsize = all_memory / 1024.0 / 1024.0
        results.append(view_data)
    return results

def writeCsvFile(fn, results) :
    if len(results) == 0:
        return
    outputfile = getPathName(fn) + "result.csv"
    filename = ""
    try:
        with open(outputfile, 'w', newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(['no', 'view', "memsize, details"])
            viewname = memsize = details = ""
            i = 0
            for view_data in results:
                i = i + 1
                viewname = view_data.name
                memsize = view_data.memsize
                for info in view_data.details:
                    details +=  str("original width:{0},original height:{1},width:{2},height:{3}, size:{4}".format(info.orgin_width, info.orgin_height, info.width,info.height, info.size)) + "\n"
                writer.writerow([i, viewname, memsize, details])
                viewname = memsize = details = ""
            csvfile.close()
    except Exception as err:
        print("file %s write results failed:[%s]" % (filename, err))

if __name__ == '__main__':
    if len(sys.argv) == 2:
        pass
    else:
        #sys.exit(HELP_STR)
        results = analysis("")
        writeCsvFile("", results)
        pass
    if input("finished, press any key to exit... ... ..."):
        pass
