#ifndef TCP_CLIENT_LIST_H
#define TCP_CLIENT_LIST_H

#include "tcp_client.h"

struct tcp_callback_list {
	CB_FUNCTION callback;
	struct tcp_callback_list *next;
};

void init_tcp_callback_list(struct tcp_callback_list **list);
void add_tcp_callback_list(struct tcp_callback_list *list, CB_FUNCTION callback);
void call_tcp_callback_list(struct tcp_callback_list *list, struct info_t *info);
void remove_tcp_callback_list(struct tcp_callback_list *list, CB_FUNCTION callback);


#endif
