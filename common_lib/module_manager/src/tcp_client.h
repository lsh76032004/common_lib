#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>

#define TCP_BUFFER	1024
#define IPADDR_SIZE	32

enum callback_condition {
	START_MONITOR = 0,
	STOP_MONITOR,
	RECV_DATA,
	SEND_DATA,
	SEND_FAILED,
	MAX_CALLBACK_CONDITION,
};

struct tcp_callback_list;

struct info_t {
	char server_name[IPADDR_SIZE];
	int server_port;

	struct sockaddr_in server_address;
	int sock;

	pthread_t tid;
	bool is_run;
	bool stop;

	char receive_msg[TCP_BUFFER];
	char send_msg[TCP_BUFFER];

	int (*send)(struct info_t *info, const char *msg);

	struct tcp_callback_list *callback_list[MAX_CALLBACK_CONDITION];
};

typedef void* (*CB_FUNCTION)(struct info_t *);

struct tcp_client_t {
	int (*open)(struct tcp_client_t *client, const char *server_name, int server_port);
	int (*wait)(struct tcp_client_t *clinet);
	int (*close)(struct tcp_client_t *clinet);
	int (*connect)(struct tcp_client_t *client);
	int (*set_callback)(struct tcp_client_t *client, int condition, 
			CB_FUNCTION cb);
	int (*unset_callback)(struct tcp_client_t *client, int condition,
			CB_FUNCTION cb);

	int (*run)(struct tcp_client_t *client);
	int (*stop)(struct tcp_client_t *client);

	struct info_t info;
};

struct tcp_client_t* tcp_init_client();
void                 tcp_terminate_client(struct tcp_client_t *client);

#endif
