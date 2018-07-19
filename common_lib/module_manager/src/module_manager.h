#ifndef MODULE_MANAGER_H
#define MODULE_MANAGER_H

#include <stdbool.h>
#include "tcp_client.h"

struct settings_t settings;

struct settings_t {
	char **argv;
	bool thread;
	bool watchdog;
	bool logcat;
	bool console;
	bool verbose;
	int port;
	char address[IPADDR_SIZE];
};

#endif
