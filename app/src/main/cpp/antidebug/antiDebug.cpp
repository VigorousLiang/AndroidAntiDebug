#include "antiDebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/ptrace.h>
#include <Android/log.h>

extern "C"

// 自定义的LOG的标识
#define TAG "antiDebug"
// 定义LOGI类型
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__)
using namespace std;
AntiDebug::AntiDebug() {

}
/**
 * 初始化并调用反调试的各种方法
 * 目前demo中尚未添加杀死进程的逻辑
 * @param env
 * @return
 */
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
    if (!isCurrentProcessNameCorrect()) {
        return true;
    }
    if (!checkRunningEnvironment()) {
        return true;
    }
    if (!checkTracePid()) {
        return true;
    }
    if (!checkPtraceStatus()) {
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
/**
 * 判断当前是否存在调试进程
 * @return
 */
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
/**
 * 判断当前进程的父进程是否为原生的zygote
 * @return
 */
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
/**
 * 判断当前的进程名称是否为预设的进程名称
 * demo中预设的名称为com.vigorous.android.antidebug
 * @return
 */
bool AntiDebug::isCurrentProcessNameCorrect() {
    // 设置buf
    pid_t pid = getpid();
    stringstream pidSS;
    pidSS << pid;
    string strPid = pidSS.str();
    FILE* pfile = NULL;
    char buf[0x1000] = { 0 };
    pfile = popen("ps", "r");
    if (NULL == pfile) {
        LOGI("isCurrentProcessNameCorrect file could not find");
        return true;
    }
    LOGI("isCurrentProcessNameCorrect: pid %s", strPid.c_str());
    while (fgets(buf, sizeof(buf), pfile)) {
        char* charPid = NULL;
        charPid = strstr(buf, strPid.c_str());
        if (charPid) {
            char* strProcessName = NULL;
            strProcessName = strstr(buf, defaultProcessName);
            if (strProcessName) {
                pclose(pfile);
                LOGI("isCurrentProcessNameCorrect true");
                return true;
            } else {
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
/**
 * 检测运行进程环境中的线程数
 * @return true 环境正常  false 环境异常
 */
bool AntiDebug::checkRunningEnvironment() {
    char buf[0x100] = { 0 };
    char* str = "/proc/%d/task";
    snprintf(buf, sizeof(buf), str, getpid());
    // 打开目录:
    DIR* pdir = opendir(buf);
    if (!pdir) {
        LOGI("checkRunningEnvironment open() fail.");
        return true;
    }
    // 查看目录下文件个数:
    struct dirent* pde = NULL;
    int count = 0;
    while ((pde = readdir(pdir))) {
        // 字符过滤
        if ((pde->d_name[0] <= '9') && (pde->d_name[0] >= '0')) {
            ++count;
            LOGI("NO.%d thread name:%s", count, pde->d_name);
        }
    }
    LOGI("thread count：%d", count);
    if (count <= 1) {
        // 此处判定为调试状态.
        LOGI("checkRunningEnvironment false");
        return false;
    }
    return true;
}
/**
 * 检测ptrace状态,尝试去Ptrace自己
 * @return true 环境正常  false 环境异常
 */
bool AntiDebug::checkPtraceStatus() {
    // ptrace如果被调试返回值为-1，如果正常运行，返回值为0
    int iRet = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    //某些手机例如小米在进行ptrace时明明当前tracePid为0，但返回值为-1.
    //在root过的华为荣耀6上测试可以。
    //原因尚待调查
    if (-1 == iRet) {
        LOGI("ptrace失败，进程正在被调试");
        return false;
    } else {
        LOGI("ptrace的返回值为:%d", iRet);
        return true;
    }
}
/**
 * 检测tracePid的值
 * @return true 正常未被调试  false 正在被调试
 */
bool AntiDebug::checkTracePid() {
    try {
        const int bufsize = 1024;
        char filename[bufsize];
        char line[bufsize];
        int pid = getpid();
        sprintf(filename, "/proc/%d/status", pid);
        FILE* fd = fopen(filename, "r");
        if (fd != nullptr) {
            while (fgets(line, bufsize, fd)) {
                if (strncmp(line, "TracerPid", 9) == 0) {
                    int statue = atoi(&line[10]);
                    LOGI("%s", line);
                    //若当前tracePid不为0或者当前pid(防止有守护进程)
                    if (statue != 0 && statue != pid) {
                        LOGI("be attached !! kill %d", pid);
                        fclose(fd);
                        //若需要直接杀死当前进程，把下注释代码打开即可
                        //int ret = kill(pid, SIGKILL);
                        return false;
                    }
                    break;
                }
            }
            if (fd != nullptr) {
                fclose(fd);
            }
        } else {
            LOGI("open %s fail...", filename);
        }
    } catch (...) {

    }
    return true;
}
