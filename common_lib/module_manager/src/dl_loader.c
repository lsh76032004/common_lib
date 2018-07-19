#include <stdio.h>
#include <string.h>
#include "watchdog.h"
#include "logcat.h"
#include "dl_loader.h"

void dl_load(struct dl_info_t *info)
{
	CB_FUNCTION dl_func = NULL;
	char dl_path[DL_NAME];

	struct dl_info_t *iter_file;
	struct dl_func_info_t *iter_func;
	void *handler;

	if (info == NULL) {
		return;
	}

	strncpy(dl_path, DL_SO_PATH, DL_NAME);

	for (iter_file = info->next; iter_file != info; iter_file = iter_file->next) {
		strncat(dl_path, iter_file->filename, DL_NAME);

		handler = dlopen (dl_path, RTLD_LAZY);
		if (!handler) {
			log_e("No dl!(%s)\n", dl_path);
		}

		for (iter_func = iter_file->dl_func->next; iter_func != iter_file->dl_func; iter_func = iter_func->next) {
			dl_func = dlsym(handler, iter_func->func);
			if (dlerror() != NULL)  {
				log_e("No dl func!(%s)\n", iter_func->func);
			}

			iter_func->cb = dl_func;
			if (strcmp(iter_func->cond, "RECEIVE_DATA") == 0)
				iter_func->condition = RECV_DATA;
			else if (strcmp(iter_func->cond, "SEND_DATA") == 0)
				iter_func->condition = SEND_DATA;
			else if (strcmp(iter_func->cond, "START_MONITOR") == 0)
				iter_func->condition = START_MONITOR;
			else if (strcmp(iter_func->cond, "STOP_MONITOR") == 0)
				iter_func->condition = STOP_MONITOR;
			else
				iter_func->condition = -1;
		}
	}
}

void dl_register_callback(struct tcp_client_t *client, struct dl_info_t *info)
{
	char dl_path[DL_NAME];

	struct dl_info_t *iter_file;
	struct dl_func_info_t *iter_func;
	int num_of_file = 0;
	int num_of_func = 0;
	
	if (info == NULL) {
		return;
	}

	strncpy(dl_path, DL_SO_PATH, DL_NAME);

	for (iter_file = info->next; iter_file != info; iter_file = iter_file->next) {
		num_of_file++;
		for (iter_func = iter_file->dl_func->next; iter_func != iter_file->dl_func; iter_func = iter_func->next) {
			num_of_func++;
			client->set_callback(client, iter_func->condition, iter_func->cb);
		}
	}

	watchdog.num_of_dl_files = num_of_file;
	watchdog.num_of_dl_funcs = num_of_func;
}
