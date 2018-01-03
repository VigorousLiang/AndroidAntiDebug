
#ifndef _ANTIDEBUG_H_
#define _ANTIDEBUG_H_

#include <jni.h>

typedef uint8_t u8;
typedef uint32_t u32;

class AntiDebug{

public:
    AntiDebug();

    bool anti_debug(JNIEnv *env);
    bool checkBreakPointCMD(u8* addr,u32 size);

private:
    bool isPort23946Open();
    bool isDebugProcessExist();
    bool isParentZygote();
    bool isCurrentProcessNameCorrect();
    bool checkRunningEnvironment();
    bool checkPtraceStatus();
    bool checkTracePid();
    const char* defaultProcessName = "com.vigorous.android.antidebug";
};
#endif