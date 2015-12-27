#！
import subprocess
import os
import re #regular express module
############################################class defination ####################################################
class CodeCounting():
    def __init__(self):
        self.comData = CommonData()
        self.tempoGuistoResult = []

    def setCommonData(self, commData):
        self.comData = commData

    def run(self):
        all_result = []

        print("dealing with all braches... ...")
        #process nena branch
        all_result.append(self.deal_branch(BranchNena(self.comData)))

        #process delphi pranch
        all_result.append(self.deal_branch(BranchDelphi(self.comData)))

        #nais config
        all_result.append(self.deal_branch(BranchNaisConfig(self.comData)))

        #TempoGuisuto
        self.tempoGuistoResult = self.deal_branch(BranchTempoGuiusto(self.comData))
        all_result.append(self.tempoGuistoResult)

        #do some refinement
        logResult = self.refine_results(all_result)

        #do statistic job
        self.do_statistic(logResult)

        #outbut logs
        self.output_LogReuslt(logResult)

        print(" ---finished--- ")
        if input("press any key to exit... ... ..."):
            pass

    ########################################################################################
    #do statistic work
    ########################################################################################
    def do_statistic(self,logResult):
        #sum insert lines and delete lines
        sum_add = 0
        sum_del = 0
        for row in logResult.rows:
            if len(row.m_insert) > 0:
                sum_add += int(''.join(filter(str.isdigit, row.m_insert)))
            if len(row.m_delete) > 0:
                sum_del += int(''.join(filter(str.isdigit, row.m_delete)))
        logResult.m_sum_add = sum_add
        logResult.m_sum_del = sum_del


        #summarize insert/dele lines of every user
        commData = self.comData
        for username in commData.username:
            sum_add = 0
            sum_del = 0
            for row in logResult.rows:
                if row.m_user == username:
                    if len(row.m_insert) > 0:
                        sum_add += int(''.join(filter(str.isdigit, row.m_insert)))
                    if len(row.m_delete) > 0:
                        sum_del += int(''.join(filter(str.isdigit, row.m_delete)))
            logResult.m_user_data[username]= (sum_add,sum_del)

        #summarize insert/dele lines of every group
        for (groupKey,groupVlaue) in commData.groups.items():
            sum_add = 0
            sum_del = 0
            for (usrDataKey,usrDataVal) in logResult.m_user_data.items():
                if usrDataKey in groupVlaue:
                    sum_add += usrDataVal[0]
                    sum_del += usrDataVal[1]
            logResult.m_group_data[groupKey]= [sum_add,sum_del]

        #deal with TempoGuisuto separately
        '''
        for row in self.tempoGuistoResult.rows:
            if len(row.m_insert) > 0:
                sum_add += int(''.join(filter(str.isdigit, row.m_insert)))
            if len(row.m_delete) > 0:
                sum_del += int(''.join(filter(str.isdigit, row.m_delete)))
        logResult.m_group_data["TempoGiusto"]= (sum_add,sum_del)
        '''

    def output_LogReuslt(self,logResult):
        commData = self.comData
        #output rows
        try:
            fi = open(commData.outfilename, 'w')
            #output first line
            fi.writelines("Group,Name,Date,Subject,Change files,Insertion,Deletion,ChangeId\n")
            for row in logResult.rows:
                fi.writelines(commData.getGroupName(row.m_user) + "," + row.m_user + "," \
                            + row.m_date + "," + row.m_subject + "," + row.m_changeFiles \
                            + "," + row.m_insert + "," + row.m_delete + "," + row.m_changeId + "\n")
            fi.writelines("sum add: ," + str(logResult.m_sum_add) + "\n")
            fi.writelines("sum delte: ," + str(logResult.m_sum_del))
            #output summarize information
            fi.writelines("\n")
            fi.writelines("\n")
            fi.writelines("------user statistical information------\n")
            fi.writelines("Name,Insertion,Deletion\n")
            for (key,value) in logResult.m_user_data.items():
                fi.writelines(key + "," + str(value[0]) + "," + str(value[1]) +"\n")

            fi.writelines("------Group statistical information------\n")
            fi.writelines("Module,Insertion,Deletion\n")
            for (key,value) in logResult.m_group_data.items():
                fi.writelines(key + "," + str(value[0]) + "," + str(value[1]) +"\n")
            fi.close()
        except Exception as err:
            print(err)

############################################  other methods #############################################################

    ########################################################################################
    #refine_results
    ########################################################################################
    def refine_results(self,allBranchResults):
        logResult = LogResult()

        #merge all data to rows
        rowlist = []
        for branchResult in allBranchResults:
            if branchResult.m_valid:
                rowlist += branchResult.rows

        #remove duplicate changeids from rowlist
        tmpRowMap = {}
        for row in rowlist:
            if not row.m_changeId in tmpRowMap:
                tmpRowMap[row.m_changeId] = row
            else:
                logResult.m_duplicaeChangeIds.append(row.m_changeId)
        logResult.rows += tmpRowMap.values()

        #test code to check the result before remove duplicates
        #logResult.rows = rowlist
        return logResult

    ########################################################################################
    #deal_branch return BranchResult
    ########################################################################################
    def deal_branch(self,branchinfo):
        results=[]
        nenabasedir = branchinfo.basedir + branchinfo.branchModules[0]
        for i, module in enumerate(branchinfo.branchModules) :
            if i == 0:
                print(nenabasedir)
                moduleResult = self.deal_moudel(branchinfo, nenabasedir)
                if moduleResult.m_valid:
                    results.append(moduleResult)
            else:
                pass
                print(nenabasedir + "/" + module)
                moduleResult = self.deal_moudel(branchinfo, nenabasedir + "/" + module)
                if moduleResult.m_valid:
                    results.append(moduleResult)

        #deal with the branch results
        return self.deal_moduel_results(results)

    ########################################################################################
    # output log
    ########################################################################################
    def output_log(self,filename,results):
        fi = open(filename, 'w')
        for item in results:
            fi.writelines(item)

        fi.close()

    ########################################################################################
    # deal with results of module
    ########################################################################################
    def deal_moduel_results(self, results):
        branch_reslut = BranchResult()
        cmmData = self.comData

        for moduleResult in results:
            if moduleResult.m_valid:
                branch_reslut.m_modules.append(moduleResult.m_path)

                #slit to all change info
                changeLogList = moduleResult.m_log.split(cmmData.splitStr[0])
                list1= []

                for changeLog in changeLogList:
                    if len(changeLog.strip()) > 0:
                        rowData = RowData()
                        changeLog = cmmData.splitStr[0] + changeLog

                        #parse changeid
                        changeId = regSearch(changeLog,cmmData.regstr["changeid"])
                        rowData.m_changeId = changeId.lstrip(cmmData.splitStr[0]).strip()
                        if len(rowData.m_changeId) == 0:
                            continue

                        #parse insert/delete lines
                        #todo how to improve the regulare expression
                        regstr = cmmData.regstr["stat_all"] + "|" + cmmData.regstr["stat_insert"] \
                                + "|" + cmmData.regstr["stat_del"] + "|" + cmmData.regstr["stat_none"]
                        stat = regSearch(changeLog,regstr)
                        if len(stat) == 0:
                            print("[%s] no insert/delete lines" % rowData.m_changeId)
                            #print(changeLog)
                            continue
                        detailedStat = self.parseStatinfo(stat)
                        rowData.m_changeFiles = detailedStat[0]
                        rowData.m_insert = detailedStat[1]
                        rowData.m_delete = detailedStat[2]

                        #parse users
                        user = regSearch(changeLog,cmmData.regstr["user"])
                        #if len(user) == 0:
                        #    print(changeLog)
                        rowData.m_user = "".join(user.split("%"))

                        #parse subject
                        subject = regSearch(changeLog,cmmData.regstr["subject"])
                        #if len(user) == 0:
                        #    print(changeLog)
                        title = "".join(subject.split("Title:"))
                        if "," in title:
                            title = '.'.join(title.split(","))
                        rowData.m_subject = title

                        #parse date
                        date = regSearch(changeLog,cmmData.regstr["date"])
                        #if len(user) == 0:
                        #    print(changeLog)
                        rowData.m_date = date

                        #add rows
                        branch_reslut.rows.append(rowData)
                        branch_reslut.m_valid = True
                    else:
                        pass

        return branch_reslut

    ########################################################################################
    # deal with module
    ########################################################################################
    def deal_moudel(self,branchinfo,path):

        #execute commands
        result = ModuleResult()
        result.m_log = self.performCmd(branchinfo.cmd_git_log,path)

        if len(result.m_log) > 0:
            result.m_valid = True
            result.m_path = path
        else:
            pass
            #print("unchanged module:%s \n,%s" % (path,result.m_stat))

        return result

    #parse insert/delete lines string
    # 1 file changed, 1 insertion(+), 1 deletion(-)
    def parseStatinfo(self,stat):
        result = []
        cmmdata = self.comData
        #split with ","
        stat_list = deal_string(stat,cmmdata.splitStr[3])
        result.append(''.join(stat_list[0:1]))
        strFirst = ''.join(list(stat_list[1:2]))
        strSecond = ''.join(stat_list[2:3])
        strIns = ""
        strDel = ""
        if 'insertion' in strFirst:
            strIns = strFirst
        elif 'deletions' in strFirst:
            strDel = strFirst

        if 'deletion' in strSecond:
            strDel = strSecond
        elif 'insertions' in strSecond:
            strIns = strSecond

        if len(strIns) == 0:
            strIns = " 0 insertion(+)"

        if len(strDel) == 0:
            strDel = " 0 deletion(-)"

        result.append(strIns)
        result.append(strDel)

        return result

    ########################################################################################
    # perform git log command
    ########################################################################################
    def performCmd(self, gitLogCmd, path):
        #print(gitLogCmd)
        result = ''
        try:
            os.chdir(path)
            proc = subprocess.Popen(gitLogCmd,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE)
            stdout, stderr = proc.communicate()

            if type(stdout) == bytes:
                result = stdout.decode("utf-8");
            if type(stdout) == str:
                result = stdout

        except Exception as err:
            print("os err:", err)

        return result
############################################  other methods #############################################################

class CommonData():
    def __init__(self):
        #check datae
        self.dateBefore = "2015-12-01"
        self.dateAfter = "2015-11-01"
        #authors
        self.author = ["zhou.g","yibq","jia.xy","wang.bing_neu","sui_x","ma.j", "meng-fw",
                  "zhang-shu","liu-yufeng"]

        self.username = [""]

        self.groups = {"DI": (""),
                      "GV": (""),
                      "Guidance" : (""),
                      "Route" : ("")}
                      #"TempoGiusto" : "TempoGiusto"} not supported so far

        #chec type, %an-author, %ci time, %s title, shortstat insert/delete coude cont, %b detailed
        self.logOption =["--pretty=format:'%an'", "--pretty=format:'%ci'", "--pretty=format:'%s'", "--shortstat", "--pretty=format:%b"]

        #defining regular express or split string
        self.splitStr = ["Change-Id:","%", "\n", ","]

        self.regstr = {"changeid" : "Change-Id:[\s]?[\w]+",
                        "user" : "%\w+[.,-]?\w+%?",
                        "date" : "\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2}\s[+]\d+",
                        #"stat" : "\d+\s+files?\s+changed\,\s+\d+\s+insertions?\(\+\),\s\d+\s+deletions?\(\-\)",
                        ##todo how to use one regular expression##
                        "stat_all" : "\d+\s+files?\s+changed,\s+\d+\s+insertions?\(\+\),\s+\d+\s+deletions?\(\-\)",
                        "stat_insert" : "\d+\s+files?\s+changed,\s+\d+\s+insertions?\(\+\)",
                        "stat_del" : "\d+\s+files?\s+changed,\s+\d+\s+deletions?\(\-\)",
                        "stat_none" : "\d+\s+files?\s+changed",
                        "subject" : "Title:.+"}

        self.cmd_git_log = ["git",
                       "log",
                       "--after=" + self.dateAfter,
                       "--before="+ self.dateBefore,
                       "--no-merges",
                       "--shortstat",
                       '--pretty=format:" %b %%%an%% %ci Title:%s%n "']
        #folder
        self.basedir = ""#"F:/SourceCode/"

        #output file
        self.outfilename = "" #"F:/SourceCode/aa.csv"

    def setBasePath(self,path):
        self.basedir = path

    def setOutputFile(self,filename):
        self.outfilename = filename

    def setBeforeDate(self, dateBefore):
        self.dateBefore = dateBefore

    def setAfterDate(self, dateAfter):
        self.dateAfter = dateAfter

    def getGroupName(self,userName):
        for (key,value) in self.groups.items():
            if userName in value:
                return key
        return ""

class BranchBase():
    def __init__(self,commData):

        self.basedir = commData.basedir

        #all modules
        self.branchModules = []

        #template for git log
        users =["--author=" + name for name in commData.author]

        #preparing commands
        self.cmd_git_log = commData.cmd_git_log[0:2] + users + commData.cmd_git_log[2:]

class BranchNena(BranchBase):
    def __init__(self,commData):
        #integration/delphi_ferrari
        super(BranchNena, self).__init__(commData)
        #integration nena
        self.branchModules = [""]

class BranchDelphi(BranchBase):
    def __init__(self,commData):
        super(BranchDelphi, self).__init__(commData)
        #integration/delphi_ferrari
        self.branchModules = [""]

class BranchNaisConfig(BranchBase):
    def __init__(self,commData):
        super(BranchNaisConfig, self).__init__(commData)
        #nais-config
        self.branchModules = [""]

class BranchTempoGuiusto(BranchBase):
    def __init__(self,commData):
        super(BranchTempoGuiusto, self).__init__(commData)
        #TempoGiusto
        self.branchModules = [""]


class ModuleResult():
        def __init__(self):
            self.m_log = ""
            self.m_valid = False
            self.m_path = ""

class RowData():
    def __init__(self):
        self.m_user = ""
        self.m_date = ""
        self.m_subject= ""
        self.m_changeFiles = ""
        self.m_insert = ""
        self.m_delete = ""
        self.m_changeId = ""

class BranchResult():
        def __init__(self):
            self.rows = []
            self.m_modules = []  #insiginificant, only used for verification
            self.m_valid = False

class LogResult():
        def __init__(self):
            self.rows = []
            self.m_sum_add = 0
            self.m_sum_del = 0
            self.m_group_data = {}
            self.m_user_data = {}
            self.m_duplicaeChangeIds = []
            self.valid = False
############################################class defination ####################################################

########################################################################################
# string methods
########################################################################################
def deal_string(string, splitstr):
    results = []
    for item in string.split(splitstr):
      if not item.isspace() and len(item) > 0:
        #print(line)
        results.append(item)
    return results

def regSearch(content,regStr):
    patten = re.compile(regStr)
    result = re.findall(patten,content)
    if len(result) == 0 :
        #print(content)
        pass
    return ("".join(result))

def do_config(commData):
    if len(commData.basedir) == 0:
        try:
            currentPath = os.getcwd()
            if not currentPath[:-1] == "/":
                currentPath = currentPath.replace("\\","/") + "/"
            commData.setBasePath(currentPath)
        except Exception as err:
            print(err)
        if len(commData.outfilename) == 0:
            commData.setOutputFile(commData.basedir + "count.csv")

if __name__ == '__main__':
    commData = CommonData()
    do_config(commData)
    codeContObj = CodeCounting()
    codeContObj.setCommonData(commData)
    codeContObj.run()


