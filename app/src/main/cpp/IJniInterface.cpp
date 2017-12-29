#include <jni.h>
#include "antidebug/antiDebug.h"
#include <string.h>

/*
 * Method:    initJNIEnv
 */
jboolean Jni_iJNIE(JNIEnv *pEnv, jclass msgFactoryClass) {
    AntiDebug* antiDebug=new AntiDebug();
    bool result=antiDebug->anti_debug(pEnv);
    if(result){
        return JNI_TRUE;
    }else{
        return JNI_FALSE;
    }
}

/**
 * 方法对应表
 */
static JNINativeMethod gMethods[] = { { "iJNIE", "()Z", (void*) Jni_iJNIE }, };

/*
 * 为某一个类注册本地方法
 */
static int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * 为所有类注册本地方法
 */
static int registerNatives(JNIEnv* env) {
    const char* kClassName =
            "com/vigorous/android/antidebug/nativeInterface/IJniInterface"; //指定要注册的类
    return registerNativeMethods(env, kClassName, gMethods,
            sizeof(gMethods) / sizeof(gMethods[0]));
}

/*
 * System.loadLibrary("lib")时调用
 * 如果成功返回JNI版本, 失败返回-1
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK) {
        return -1;
    }

    if (!registerNatives(env)) {
        return -1;
    }
    result = JNI_VERSION_1_4;

    return result;
}
