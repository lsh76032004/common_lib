#ifndef DL_LOADER_H
#define DL_LOADER_H

#include <dlfcn.h>
#include "dl_parser.h"
#include "tcp_client.h"

void dl_load(struct dl_info_t *info);
void dl_register_callback(struct tcp_client_t *client, struct dl_info_t *info);

#endif
