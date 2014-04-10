#ifdef __ANDROID__
#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, __VA_ARGS__)
#endif
