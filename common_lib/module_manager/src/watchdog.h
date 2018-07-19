#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <pthread.h>
#include "tcp_client.h"

struct cpu_usage_t {
	float usage;
	unsigned long long prev_total_tick;
	unsigned long prev_pid_tick;
};

struct mem_usage_t {
	unsigned int curr_real;
	unsigned int peak_real;
	unsigned int curr_virt;
	unsigned int peak_virt;
};

struct watchdog_t {
	pid_t pid;
	pthread_t tid;

	int num_of_recv_msg;
	int num_of_recv_failed;
	int num_of_send_msg;
	int num_of_send_failed;
	long byte_of_recv_msg;
	long byte_of_send_msg;

	int num_of_dl_files;
	int num_of_dl_funcs;

	struct cpu_usage_t cpu;
	struct mem_usage_t mem;

	struct info_t *info;

	unsigned int watchdog_intval;
};

struct watchdog_t watchdog;

void watchdog_init();
int watchdog_run();
void* watchdog_monitor_tcp_send_cb(struct info_t *info);
void* watchdog_monitor_tcp_recv_cb(struct info_t *info);
void* watchdog_monitor_tcp_send_failed_cb(struct info_t *info);

#endif
