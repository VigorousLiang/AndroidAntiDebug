#include "antiDebug.h"
#include <stdio.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <Android/log.h>
extern "C"

// 自定义的LOG的标识
#define TAG "antiDebug"
// 定义LOGI类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
using namespace std;
AntiDebug::AntiDebug() {

}
bool AntiDebug::anti_debug(JNIEnv *env) {
    if (isPort23946Open()) {
        return true;
    }
    if (isDebugProcessExist()) {
        return true;
    }
    if (!isParentZygote()) {
        return true;
    }
    if(!isCurrentProcessNameCorrect()){
        return true;
    }
    return false;
}
/**
 * 检测Android_server常用的23946端口是否开启
 * @return
 */
bool AntiDebug::isPort23946Open() {
    FILE* pfile = NULL;
    char buf[0x1000] = { 0 };
    // 执行命令
    char* strCatTcp = "cat /proc/net/tcp |grep :5D8A";
    pfile = popen(strCatTcp, "r");
    if (NULL == pfile) {
        LOGI("isPort23946Open file could not find");
        return false;
    }
    // 获取结果
    while (fgets(buf, sizeof(buf), pfile)) {
        // 执行到这里，判定为调试状态
        LOGI("isPort23946Open find debug");
        return true;
    } //while
    pclose(pfile);
    LOGI("isPort23946Open does not find debug");
    return false;
}

bool AntiDebug::isDebugProcessExist() {
    FILE* pfile = NULL;
    char buf[0x1000] = { 0 };
    pfile = popen("ps", "r");
    if (NULL == pfile) {
        LOGI("isDebugProcessExist file could not find");
        return false;
    }
    while (fgets(buf, sizeof(buf), pfile)) {
        // 查找子串
        char* strA = NULL;
        char* strB = NULL;
        char* strC = NULL;
        char* strD = NULL;
        strA = strstr(buf, "android_server");
        strB = strstr(buf, "gdbserver");
        strC = strstr(buf, "gdb");
        strD = strstr(buf, "fuwu");
        if (strA || strB || strC || strD) {
            pclose(pfile);
            LOGI("isDebugProcessExist find debug");
            return true;
        }
    }
    pclose(pfile);
    LOGI("isDebugProcessExist does not find debug");
    return false;
}
bool AntiDebug::isParentZygote() {
    // 设置buf
    char strPpidCmdline[0x100] = { 0 };
    snprintf(strPpidCmdline, sizeof(strPpidCmdline), "/proc/%d/cmdline",
            getppid());
    // 打开文件
    int file = open(strPpidCmdline, O_RDONLY);
    if (file < 0) {
        //CheckParents open错误!
        LOGI("isParentZygote file could not find");
        return true;
    }
    // 文件内容读入内存
    memset(strPpidCmdline, 0, sizeof(strPpidCmdline));
    ssize_t ret = read(file, strPpidCmdline, sizeof(strPpidCmdline));
    if (-1 == ret) {
        //CheckParents read错误!
        LOGI("isParentZygote file read error");
        return true;
    }
    // 没找到返回0
    char* sRet = strstr(strPpidCmdline, "zygote");
    if (NULL == sRet) {
        // 执行到这里，判定为调试状态
        LOGI("isParentZygote false");
        return false;
    }
    LOGI("isParentZygote true");
    return true;
}
bool AntiDebug::isCurrentProcessNameCorrect() {
    // 设置buf
    pid_t pid = getpid();
    stringstream pidSS;
    pidSS<<pid;
    string strPid = pidSS.str();
    FILE* pfile = NULL;
    char buf[0x1000] = { 0 };
    pfile = popen("ps", "r");
    if (NULL == pfile) {
        LOGI("isCurrentProcessNameCorrect file could not find");
        return true;
    }
    LOGI("isCurrentProcessNameCorrect: pid %s",strPid.c_str());
    while (fgets(buf, sizeof(buf), pfile)) {
        char* charPid = NULL;
        charPid = strstr(buf, strPid.c_str());
        if (charPid){
            char* strProcessName=NULL;
            strProcessName= strstr(buf, defaultProcessName);
            if(strProcessName){
                pclose(pfile);
                LOGI("isCurrentProcessNameCorrect true");
                return true;
            }else{
                pclose(pfile);
                LOGI("isCurrentProcessNameCorrect false");
                return false;
            }
        }
    }
    pclose(pfile);
    LOGI("isCurrentProcessNameCorrect false");
    return false;
}

