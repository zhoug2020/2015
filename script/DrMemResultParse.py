#！
import subprocess
import os
import re #regular express module
import csv
########################################################################################
def getCurrentPath():
    try:
        currentPath = os.getcwd()
        if currentPath[:-1] != os.path.sep:
            currentPath = currentPath + os.path.sep
        return currentPath
        pass
    except Exception as err:
        print(err)
    pass
########################################################################################
def readresults(path) :
    src = path + "results.txt"
    try:
        fp = open(src, "r")
        lines = fp.readlines()
        fp.close()
        return lines
    except Exception as err:
        print("file %s read results failed:[%s]" % (src, err))
        pass

    pass
########################################################################################
def regSearch(content,regStr):
    patten = re.compile(regStr)
    result = re.findall(patten,content)
    if len(result) == 0 :
        #print(content)
        pass
    return ("".join(result))

########################################################################################
def deal_string(string, splitstr):
    results = []
    for item in string.split(splitstr):
      if not item.isspace() and len(item) > 0:
        #print(line)
        results.append(item)
    return results

########################################################################################
def writeresults(path, lines) :
    src = path + "results.csv"
    try:
        with open('csv_test.csv', 'w',newline='') as csvfile:
            writer = csv.writer(csvfile)
            writer.writerow(["no", "ErrNo", "ErrType", "ErrDescription", "ErrStack", "Errmodue", "Errcode"])
            errlinecnt = 0
            i = 0
            startflag = False
            errDesc = errDetails = erroNo = errModule = errCode = errType = ""
            for line in lines:
                if (len(regSearch(line, "^Error #")) > 0):
                    if errlinecnt > 0:
                        i = i + 1
                        #write line
                        writer.writerow([i,erroNo, errType, errDesc, errDetails, errModule, errCode])
                        errDesc = errDetails = erroNo = errModule = errCode = errType = ""
                        errlinecnt = 0
                     #print(detais)
                    summary = deal_string(line, ':')
                    if (len(summary) > 0):
                        erroNo = ''.join(list(summary[0:1]))
                        #print(erroNo)
                        tmpline =  ''.join(list(summary[1:]))
                        tmpline = tmpline.strip()
                        if tmpline.find("UNADDRESSABLE") != -1:
                            errType = "UNADDRESSABLE ACCESS"
                        elif tmpline.find("UNINITIALIZED") != -1:
                            errType = "UNINITIALIZED READ"
                        elif tmpline.find("LEAK") == 0:
                            errType = "LEAK"
                        elif tmpline.find("POSSIBLE LEAK") == 0:
                            errType = "POSSIBLE LEAK"
                        else:
                            errType = ""
                        errDesc = ''.join(list(summary[-1])).strip()
                        startflag = True
                elif startflag:
                    errlinecnt += 1
                    if (errlinecnt == 1):
                        errModule = line
                    elif errlinecnt == 2:
                        errCode = line
                    errDetails += line
                pass
            csvfile.close()
    except Exception as err:
     print("file %s write results failed:[%s]" % (src, err))
     pass
########################################################################################
if __name__ == '__main__':
    lines = readresults(getCurrentPath())
    writeresults(getCurrentPath(), lines)
    if input("press any key to exit... ... ..."):
        pass