#include <stdio.h>
#include <stdlib.h>
#include "tcp_client_list.h"
#include "module_manager.h"
#include "logcat.h"

void init_tcp_callback_list(struct tcp_callback_list **list)
{
	*list = (struct tcp_callback_list*)malloc(sizeof(struct tcp_callback_list));
	if (*list == NULL) {
		return;
	}

	(*list)->callback = NULL;
	(*list)->next = (*list);
}

void add_tcp_callback_list(struct tcp_callback_list *list, CB_FUNCTION callback)
{
	struct tcp_callback_list *cb = (struct tcp_callback_list*)
		malloc(sizeof(struct tcp_callback_list));
	if (cb == NULL) {
		return;
	}

	cb->callback = callback;

	cb->next = list->next;
	list->next = cb;
}

void call_tcp_callback_list(struct tcp_callback_list *list, struct info_t *info)
{
	struct tcp_callback_list *iter;
	pthread_t tid;
	for(iter = list->next; iter != list; iter = iter->next) {
		if (iter->callback) {
			if (settings.thread) {
				pthread_create(&tid, NULL, iter->callback, (void*)info);
				log_d("Thread tid=%lu\n", tid);
			} else {
				iter->callback(info);
			}
		}
	}
}

void remove_tcp_callback_list(struct tcp_callback_list *list, CB_FUNCTION callback)
{
	struct tcp_callback_list **iter;
	struct tcp_callback_list *tmp;
	for(iter = &list->next; *iter != list; *iter = (*iter)->next) {
		if ((*iter)->callback == callback || callback == NULL) {
			tmp = *iter;
			*iter = (*iter)->next;
			free(tmp);
		}
	}
}
