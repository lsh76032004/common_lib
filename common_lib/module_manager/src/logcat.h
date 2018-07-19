#ifndef LOGCAT_H
#define LOGCAT_H

#include <stdio.h>
#include <time.h>
#include "module_manager.h"

#define LOGCAT_PATH	"/etc/3team/log"

FILE *logcat_fp;

void logcat_init();
void logcat_exit();

#define log_d(fmt, ...)		do {	\
					if (settings.verbose == false) break;	\
					if (settings.logcat == false) break;	\
					if (logcat_fp == NULL) break;		\
					time_t clk = time(NULL);		\
					struct tm *tm_info = gmtime(&clk);	\
					fprintf(logcat_fp, "%02d:%02d:%02d [%s (%s:%d)] " fmt, \
						tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, __FILE__, __func__, __LINE__,## __VA_ARGS__);	\
				} while(0);

#define log_e(fmt, ...)		do {	\
					if (settings.logcat == false) break;	\
					if (logcat_fp == NULL) break;		\
					time_t clk = time(NULL);		\
					struct tm *tm_info = gmtime(&clk);	\
					fprintf(logcat_fp, "%02d:%02d:%02d ERROR [%s (%s:%d)] " fmt, \
						tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec, __FILE__, __func__, __LINE__,## __VA_ARGS__);	\
				} while(0);

#endif
