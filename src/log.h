#ifndef LOG_H_
#define LOG_H_

#define DEBUG_ENABLED
#define ERR_ENABLED
#define INFO_ENABLED
#define WARN_ENABLED

#ifdef ERR_ENABLED
  #define logErrorF(msg, ...) cgaLog(LL_ERROR, msg, __VA_ARGS__)
  #define logError(msg) cgaLog(LL_ERROR, msg)
#else
  #define logErrorF(msg, ...)
  #define logError(msg)
#endif

#ifdef DEBUG_ENABLED
  #define logDebugF(msg, ...) cgaLog(LL_DEBUG, msg, __VA_ARGS__)
  #define logDebug(msg) cgaLog(LL_DEBUG, msg)
#else
  #define logDebugF(msg, ...)
  #define logDebug(msg)
#endif

#ifdef WARN_ENABLED
  #define logWarnF(msg, ...) cgaLog(LL_WARN, msg, __VA_ARGS__)
  #define logWarn(msg) cgaLog(LL_WARN, msg)
#else
  #define logWarnF(msg, ...)
  #define logWarn(msg)
#endif

#ifdef INFO_ENABLED
  #define logInfoF(msg, ...) cgaLog(LL_INFO, msg, __VA_ARGS__)
  #define logInfo(msg) cgaLog(LL_INFO, msg)
#else
  #define logInfoF(msg, ...) 
  #define logInfo(msg) 
#endif

typedef enum {
  LL_INFO,
  LL_DEBUG,
  LL_WARN,
  LL_ERROR
} log_level_t;

void cgaLog(log_level_t level, char* format, ...);

#endif // LOG_H_