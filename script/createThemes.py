#！/usr/bin/env python
import os,sys
import re #regular express module
########################################################################################
#functions defination
########################################################################################

########################################################################################
#get all available themes under current path
########################################################################################
def getAvalilabeThemes():
    try:
        availableThmems= {}
        currentPath = os.getcwd()
        folders = os.listdir(currentPath)
        if currentPath[:-1] != os.path.sep:
            currentPath = currentPath + os.path.sep
        for name in folders:
            if os.path.isdir(name) and re.search("theme",name,re.IGNORECASE) :
                availableThmems[name] = currentPath + name
        return availableThmems
        pass
    except Exception as err:
        print(err)
    pass

########################################################################################
#get all configuration files under given theme path
########################################################################################
def getAllConfigFiles(path):
    configFiles = []
    try:
        files = os.listdir(path)
        for fileBaseName in files:
            filePathName = path + os.path.sep + fileBaseName
            if os.path.isfile(filePathName):
                configFiles.append(fileBaseName)
        return configFiles
    except Exception as err:
        print(err)

########################################################################################
# check if the specific theme already exists.
########################################################################################
def checkConfigFile(themePath, defaultThemePath):
    if getAllConfigFiles(themePath) != getAllConfigFiles(defaultThemePath):
        return False
    return True

########################################################################################
# string methods, split string with splitstr
########################################################################################
def split_string(string, splitstr):
    results = []
    for item in string.split(splitstr):
      if not item.isspace() and len(item) > 0:
        results.append(item)
    return results

########################################################################################
# create configuration files for the specific theme
########################################################################################
def createConigFile(src,dest,themeName,encoding):
    try:
        #read source file content from default configuration file.
        fp = open(src,"r",encoding=encoding)
        lines = fp.readlines()
        fp.close()

        #parse every line and writhe to dest file
        fpout = open(dest,"w", encoding=encoding)
        for eachline in lines:
            if eachline.find("class") == 0 or \
                eachline.find("partial class") == 0 or \
                eachline.find("#include") == 0:
                fpout.writelines(eachline)
            else:
                if eachline[0].isspace():
                    if eachline.find("defaultTheme") != -1 \
                        and eachline.find("=") != -1:
                        repVal = eachline.replace("defaultTheme",themeName)
                        fpout.writelines(repVal)
                    else:
                        # option to suporot sub folder
                        '''
                        results = split_string(eachline, "=")
                        if len(results) == 2:
                            name = str(results[0]).strip();
                            value = str(results[1]).strip();
                            if (name == "backgroundImage" or name == "skin"):
                                newVal = "res/" + themeName + "/" + value
                                repVal = eachline.replace(value,newVal)
                                eachline = repVal
                        '''
                        fpout.writelines(eachline)
        fpout.close()
        return True
    except Exception as err:
        print("file %s operation failed:[%s], try to use another encoding!" % (src, err))
        return False

########################################################################################
# create the specific theme
########################################################################################
def creatTheme(themeName, themePath, defaultThemePath):
    #get the all configuration files in the default theme folder
    conFiles = getAllConfigFiles(defaultThemePath)
    for srcFile in conFiles:
        # set the filename of every destination configuration file
        destFileName = themePath + os.path.sep + srcFile
        srcFileName = defaultThemePath + os.path.sep + srcFile
        #create destination configuration file
        if not createConigFile(srcFileName,destFileName, themeName,"utf-16"):
            #try to use another file encoding
            createConigFile(srcFileName,destFileName, themeName,"utf-8")
    pass

########################################################################################
# run
########################################################################################
def run():
    #check howm many themes are there in current folder "/ui/image"
    themes = getAvalilabeThemes()
    # create themes based on default theme
    if len(themes) > 1:
        defaultTheme = themes["defaultTheme"]
        for (key,value) in themes.items():
            if key != "defaultTheme":
                #create theme file
                creatTheme(key, value, defaultTheme)
                print("%s create ok" % key)
                '''
                #create the theme if it does not exist
                if not checkConfigFile(value, defaultTheme):
                    creatTheme(key,value,defaultTheme)
                    print("%s create ok" % key)
                else:
                    print ("%s already exists" % key)
                '''
    else:
        print("only default theme exists!")

########################################################################################
#main
########################################################################################
if __name__ == '__main__':
    run()
#---------finish----------------
    print(" ---finished--- ")
    if input("press any key to exit... ... ..."):
        pass
