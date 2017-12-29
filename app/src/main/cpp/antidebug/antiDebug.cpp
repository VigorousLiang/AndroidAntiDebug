#include "antiDebug.h"
#include <stdio.h>
#include <string.h>

extern "C"

AntiDebug::AntiDebug() {

}
bool AntiDebug::anti_debug(JNIEnv *env) {
    if (isPort23946Open()) {
        return true;
    }
    if (isDebugProcessExist()) {
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
        return false;
    }
    // 获取结果
    while (fgets(buf, sizeof(buf), pfile)) {
        // 执行到这里，判定为调试状态
        return true;
    } //while
    pclose(pfile);
    return false;
}

bool AntiDebug::isDebugProcessExist(){
    FILE* pfile=NULL;
    char buf[0x1000]={0};
    pfile=popen("ps","r");
    if(NULL==pfile)
    {
        return false;
    }
    while(fgets(buf,sizeof(buf),pfile))
    {
        // 查找子串
        char* strA=NULL;
        char* strB=NULL;
        char* strC=NULL;
        char* strD=NULL;
        strA=strstr(buf,"android_server");
        strB=strstr(buf, "gdbserver");
        strC=strstr(buf,"gdb");
        strD=strstr(buf,"fuwu");
        if(strA || strB ||strC || strD)
        {
            pclose(pfile);
            return true;
        }
    }
    pclose(pfile);
    return false;
}
