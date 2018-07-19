#ifndef DL_PARSER_H
#define DL_PARSER_H

#include <stdio.h>
#include "tcp_client.h"

#define DL_PATH "/etc/3team/dl.rc"
#define DL_SO_PATH "/etc/3team/dl/"

#define DL_NAME 64
#define DL_BUFSIZ 1024

struct dl_func_info_t {
	char func[DL_NAME];
	char cond[DL_NAME];
	int condition;
	CB_FUNCTION cb;

	struct dl_func_info_t *next;
};

struct dl_info_t {
	char filename[DL_BUFSIZ];
	struct dl_func_info_t *dl_func;

	struct dl_info_t *next;
};

FILE* dl_open_file();
struct dl_info_t* dl_read_info(FILE *fp);
void dl_display_info(struct dl_info_t *info);
void dl_clear_info(struct dl_info_t *info);
void dl_close_file(FILE *fp);

#endif
