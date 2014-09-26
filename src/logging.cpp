#include "logging.h"
#include <stdio.h>

apc_log_func_ptr apclog = printf; // Default to console logging
int g_apclevel = APC_LOG_LEVEL_ERROR; // Default to only logging errors

void apc_set_log_func(apc_log_func_ptr func)
{
  if (func)
    apclog = func;
}

void apc_set_log_level(int level)
{
  g_apclevel = level;
}