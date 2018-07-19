#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>
#include "tcp_client.h"
#include "watchdog.h"
#include "logcat.h"
#include "../parson/parson.h"

#define WATCHDOG_INTVAL	30

#define WATCHDOG_BUFSIZ 1024
#define WATCHDOG_READSIZ 256

#define RPI3_CORE_NUM 	4

pthread_mutex_t watchdog_lock;

struct watchdog_t watchdog;

void watchdog_init()
{
	watchdog.pid = getpid();
	watchdog.watchdog_intval = WATCHDOG_INTVAL;
}

static void __read_cpu_usage()
{
	char line[WATCHDOG_READSIZ];
	char pid_stat[32];
	unsigned long long user, nice, system, idle;
	unsigned long long total_cpu_usage_now;
	unsigned long pid_user, pid_system;
	unsigned long pid_cpu_usage_now;
	FILE* fp;
	
	fp = fopen("/proc/stat", "r");
	if (fp == NULL) {
		log_e("fail to open /proc/stat\n");
	}

	fgets(line, sizeof(line), fp);

	sscanf(line,"%*s %llu %llu %llu %llu", &user, &nice, &system, &idle);
	total_cpu_usage_now = user + nice + system + idle;

	fclose(fp);

	sprintf(pid_stat, "/proc/%d/stat", watchdog.pid);
	fp = fopen(pid_stat, "r");
	if (fp == NULL) {
		log_e("fail to open %s\n", pid_stat);
	}

	fgets(line, sizeof(line), fp);
	sscanf(line,
			"%*d %*s %*c %*d" //pid,command,state,ppid
			"%*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu"
			"%lu %lu" //usertime,systemtime
			"%*ld %*ld %*ld %*ld %*ld %*ld %*llu"
			"%*lu", //virtual memory size in bytes
			&pid_user, &pid_system);

	pid_cpu_usage_now = pid_user + pid_system;

	if (watchdog.cpu.prev_pid_tick != 0 && watchdog.cpu.prev_total_tick != 0) {
		watchdog.cpu.usage = (pid_cpu_usage_now - watchdog.cpu.prev_pid_tick) * 100 
			/ (float) (total_cpu_usage_now - watchdog.cpu.prev_total_tick);

		watchdog.cpu.usage *= RPI3_CORE_NUM;
	}

	watchdog.cpu.prev_total_tick = total_cpu_usage_now;
	watchdog.cpu.prev_pid_tick = pid_cpu_usage_now;
}

static void __read_mem_usage()
{
	FILE* fp = fopen("/proc/self/status", "r");
	char line[WATCHDOG_READSIZ];

	// read the entire file
	while (fscanf(fp, " %1023s", line) == 1) {

		if (strcmp(line, "VmRSS:") == 0) {
			fscanf(fp, " %d", &watchdog.mem.curr_real);
		}else if (strcmp(line, "VmHWM:") == 0) {
			fscanf(fp, " %d", &watchdog.mem.peak_real);
		}else if (strcmp(line, "VmSize:") == 0) {
			fscanf(fp, " %d", &watchdog.mem.curr_virt);
		}else if (strcmp(line, "VmPeak:") == 0) {
			fscanf(fp, " %d", &watchdog.mem.peak_virt);
		}
	}
}

static void __log()
{
	log_d("[watchdog] pid=%d cpu=%f\n", 
			watchdog.pid, watchdog.cpu.usage);

	log_d("[watchdog] real=%d virt=%d\n", 
			watchdog.mem.curr_real, watchdog.mem.curr_virt);

	log_d("[watchdog] dl_files=%d dl_funcs=%d\n", 
			watchdog.num_of_dl_files, watchdog.num_of_dl_funcs);

	log_d("[watchdog] total_recv_bytes=%lu total_send_bytes=%lu\n", 
			watchdog.byte_of_recv_msg, watchdog.byte_of_send_msg);

	log_d("[watchdog] send_failed=%d\n", watchdog.num_of_send_failed);
}

static void __make_packet(char *send_buf) {
	JSON_Value *rootValue;
	JSON_Object *rootObject;
	char* buf;

	/*init empty json packet*/
	rootValue = json_value_init_object();
	rootObject = json_value_get_object(rootValue);

	/*add key & value*/
	json_object_set_string(rootObject, "pid", "watchdog");
	json_object_set_string(rootObject, "ip", "ipaddress");
	json_object_set_number(rootObject, "cpu", watchdog.cpu.usage);
	json_object_set_number(rootObject, "real_mem", watchdog.mem.curr_real);
	json_object_set_number(rootObject, "virt_mem", watchdog.mem.curr_virt);
	json_object_set_number(rootObject, "dl_files", watchdog.num_of_dl_files);
	json_object_set_number(rootObject, "dl_funcs", watchdog.num_of_dl_funcs);
	json_object_set_number(rootObject, "recv_cnt", watchdog.num_of_recv_msg);
	json_object_set_number(rootObject, "recv_byte", watchdog.byte_of_recv_msg);
	json_object_set_number(rootObject, "send_cnt", watchdog.num_of_send_msg);
	json_object_set_number(rootObject, "send_byte", watchdog.byte_of_send_msg);
	json_object_set_number(rootObject, "send_failed", watchdog.num_of_send_failed);

	/*get full string of json packet */
	buf =  json_serialize_to_string(rootValue);
	log_d("result json : %s\n", buf);
	strncpy(send_buf, buf, WATCHDOG_BUFSIZ);

	//free memory
	json_free_serialized_string(buf);
	json_value_free(rootValue);
}

static void* __run_watchdog(void *data)
{
	int intval;
	char buf[WATCHDOG_BUFSIZ];
	while (true) {
		pthread_mutex_lock(&watchdog_lock);

		intval = watchdog.watchdog_intval;

		__read_cpu_usage();
		__read_mem_usage();

		__log();
		__make_packet(buf);

		pthread_mutex_unlock(&watchdog_lock);

		watchdog.info->send(watchdog.info, buf);

		sleep(intval);
	}

	return NULL;
}

void* watchdog_monitor_tcp_send_cb(struct info_t *info)
{
	pthread_mutex_lock(&watchdog_lock);
	
	watchdog.byte_of_send_msg += strlen(info->send_msg);
	watchdog.num_of_send_msg += 1;

	pthread_mutex_unlock(&watchdog_lock);
	
	return NULL;
}

void* watchdog_monitor_tcp_send_failed_cb(struct info_t *info)
{
	pthread_mutex_lock(&watchdog_lock);
	
	watchdog.num_of_send_failed += 1;

	pthread_mutex_unlock(&watchdog_lock);
	
	return NULL;
}

void* watchdog_monitor_tcp_recv_cb(struct info_t *info)
{
	pthread_mutex_lock(&watchdog_lock);
	
	watchdog.byte_of_recv_msg += strlen(info->receive_msg);
	watchdog.num_of_recv_msg += 1;

	pthread_mutex_unlock(&watchdog_lock);

	return NULL;
}

int watchdog_run()
{
	int err;
	int ret = 0;

	if (settings.watchdog == false) {
		ret = -1;
		goto OUT;
	}

	err = pthread_create(&watchdog.tid, 0, __run_watchdog, NULL);
	if (err != 0) {
		ret = -1;
	}

	log_d("watchdog starts running\n");

OUT:
	return ret;
}
