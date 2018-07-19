#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "dl_parser.h"
#include "logcat.h"

FILE* dl_open_file()
{
	FILE* fp = fopen(DL_PATH, "r");
	if (fp == NULL) {
		log_e("Failed to open file\n");
		return NULL;
	}

	return fp;
}

#define FUNC_NAME	0
#define CONDITION	1

struct dl_info_t* dl_read_info(FILE* fp)
{
	struct dl_info_t *info = NULL;
	struct dl_info_t *tmp_info = NULL;
	struct dl_func_info_t *tmp_func = NULL;
	int c;

	bool file = false;
	bool func = false;
	bool comment = false;

	int file_name_pos = 0;
	int func_pos = 0;
	int func_option = 0;

	if (fp == NULL) {
		return NULL;
	}

	info = (struct dl_info_t*)malloc(sizeof(struct dl_info_t));
	memset(info, 0, sizeof(struct dl_info_t));
	info->next = info;

	do {
		c = fgetc(fp);

		if (c == ' ' || c == '\t') {
			if (func) {
				if (func_pos > 0) {
					func_option += 1;
					func_pos = 0;
				}
			}

			continue;
		}

		if (c == '\n') {
			comment = false;
			file_name_pos = 0;
			func_pos = 0;
			func_option = 0;
			continue;
		}

		if (c == -1) {
			break;
		}

		if (c == '[') {
			file = true;
			func = false;
			if (tmp_info) {
				tmp_info->next = info->next;
				info->next = tmp_info;
			}
			tmp_info = (struct dl_info_t*)malloc(sizeof(struct dl_info_t));
			tmp_info->dl_func = (struct dl_func_info_t*)malloc(sizeof(struct dl_func_info_t));
			memset(tmp_info->dl_func, 0, sizeof(struct dl_func_info_t));
			tmp_info->dl_func->next = tmp_info->dl_func;
			continue;
		}else if (c == ']') {
			file = false;
			tmp_info->filename[file_name_pos] = '\0';
			file_name_pos = 0;
			continue;
		}

		if (file) {
			tmp_info->filename[file_name_pos++] = c;
		}

		if (c == ':') {
			func = true;
			func_pos = 0;
			func_option = 0;

			tmp_func = (struct dl_func_info_t*)malloc(sizeof(struct dl_func_info_t));
			memset(tmp_func, 0, sizeof(struct dl_func_info_t));

			tmp_func->next = tmp_info->dl_func->next;
			tmp_info->dl_func->next = tmp_func;

			continue;
		}

		if (func) {
			switch(func_option) {
			case FUNC_NAME:
				tmp_func->func[func_pos++] = c;
				tmp_func->func[func_pos] = '\0';
				break;
			case CONDITION:
				tmp_func->cond[func_pos++] = c;
				tmp_func->cond[func_pos] = '\0';
				break;
			default:
				func = false;
				func_pos = 0;
				break;
			}
		}
	} while(!feof(fp));

	if (tmp_info) {
		tmp_info->next = info->next;
		info->next = tmp_info;
	}

	return info;
}

void dl_display_info(struct dl_info_t *info)
{
	struct dl_info_t *iter_file;
	struct dl_func_info_t *iter_func;

	if (info == NULL) {
		return;
	}

	for (iter_file = info->next; iter_file != info; iter_file = iter_file->next) {
		log_d("[file=%s]\n", iter_file->filename);
		struct dl_func_info_t *func = iter_file->dl_func;
		for (iter_func = func->next; iter_func != func; iter_func = iter_func->next) {
			log_d("func=%s, cond=%s\n", iter_func->func, iter_func->cond);
		}
	}
}

void dl_clear_info(struct dl_info_t *info)
{
	/* TODO */
	return;
}

void dl_close_file(FILE *fp)
{
	fclose(fp);


}
