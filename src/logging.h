#ifndef LOGGING_H
#define	LOGGING_H

// Logging Interface Definition
///////////////////////////////////////////////////////
enum 
{
  APC_LOG_FLAG_ERROR     = 0x1,
  APC_LOG_FLAG_WARNING   = 0x1 << 1,
  APC_LOG_FLAG_INFO      = 0x1 << 2,
  APC_LOG_FLAG_DEBUG     = 0x1 << 3
};

enum
{
  APC_LOG_LEVEL_NONE     = 0x0,
  APC_LOG_LEVEL_ERROR    = APC_LOG_FLAG_ERROR,
  APC_LOG_LEVEL_WARNING  = APC_LOG_LEVEL_ERROR | APC_LOG_FLAG_WARNING,
  APC_LOG_LEVEL_INFO     = APC_LOG_LEVEL_ERROR | APC_LOG_FLAG_WARNING | APC_LOG_FLAG_INFO,
  APC_LOG_LEVEL_DEBUG    = APC_LOG_LEVEL_ERROR | APC_LOG_FLAG_WARNING | APC_LOG_FLAG_INFO | APC_LOG_FLAG_DEBUG
};

void apc_set_log_level(int level);
typedef int (*apc_log_func_ptr)(const char* format, ...);
void apc_set_log_func(apc_log_func_ptr func);

extern apc_log_func_ptr apclog;
extern int g_apclevel;
#define APC_LOG(level,fmt, ...) if (g_apclevel & level) apclog(fmt "\n", __VA_ARGS__)
#define APC_LOG_0(level,fmt) if (g_apclevel & level) apclog(fmt "\n")

#endif	/* LOGGING_H */