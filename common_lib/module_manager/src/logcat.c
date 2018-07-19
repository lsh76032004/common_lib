#include "logcat.h"

FILE *logcat_fp;

void logcat_init()
{
	if (settings.console == false)
		logcat_fp = fopen(LOGCAT_PATH, "w");
	else
		logcat_fp = stdout;
}

void logcat_exit()
{
	if (settings.console == false)
		fclose(logcat_fp);
}
