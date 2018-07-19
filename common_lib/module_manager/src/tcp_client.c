#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/time.h>

#include "tcp_client.h"
#include "tcp_client_list.h"
#include "watchdog.h"
#include "logcat.h"

#define SEND_RECV_TIMEOUT	3

pthread_mutex_t tcp_client_lock;

static void* __run_thread(void *data)
{
	struct info_t *info = (struct info_t*)data;

	bool stop = false;
	int msg_len = 0;
	char buffer[BUFSIZ];
	char* pbuffer = buffer;

	while (!stop) {
		pthread_mutex_lock(&tcp_client_lock);

		msg_len = recv(info->sock, pbuffer, BUFSIZ, 0);
		if (msg_len > 0) {

			buffer[msg_len] = '\0';
			log_d("received: '%s'\n", buffer);

			strncpy(info->receive_msg, buffer, msg_len);

			if (info->stop) {
				info->is_run = false;
				stop = true;
			}
		}

		pthread_mutex_unlock(&tcp_client_lock);

		if (msg_len > 0) {
			call_tcp_callback_list(info->callback_list[RECV_DATA], info);
		}
	}

	return NULL;
}

static int __open_socket(struct tcp_client_t *client, const char *server_name, int server_port)
{
	int ret = 0;
	struct info_t *info = &client->info;

	strncpy(info->server_name, "192.168.0.117", IPADDR_SIZE);
	info->server_port = 33333;

	if (server_name) {
		strncpy(info->server_name, server_name, IPADDR_SIZE);
	}

	if (server_port > 0) {
		info->server_port = server_port;
	}

	memset(&info->server_address, 0, sizeof(struct sockaddr));
	info->server_address.sin_family = AF_INET;

	inet_pton(AF_INET, info->server_name, &info->server_address.sin_addr);

	info->server_address.sin_port = htons(info->server_port);

	if ((info->sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		log_e("could not create socket\n");
		ret = -1;
	}

	info->is_run = false;
	info->stop = false;

	for (int i = 0; i < MAX_CALLBACK_CONDITION; i++) {
		init_tcp_callback_list(&info->callback_list[i]);
	}

	return ret;
}

static int __wait_socket(struct tcp_client_t *client)
{
	int ret = 0;
	struct info_t *info = &client->info;

	pthread_join(info->tid, NULL);

	return ret;
}

static int __close_socket(struct tcp_client_t *client)
{
	int ret = 0;
	struct info_t *info = &client->info;

	close(info->sock);

	return ret;
}

static int __connect_socket(struct tcp_client_t *client)
{
	int ret = 0;
	struct info_t *info = &client->info;

	if (connect(info->sock, (struct sockaddr*)&info->server_address,
				sizeof(struct sockaddr_in)) < 0) {
		log_e("could not connect to server\n");
		ret = -1;
	}

	struct timeval timeout;      
	timeout.tv_sec = SEND_RECV_TIMEOUT;
	timeout.tv_usec = 0;

	if (setsockopt (client->info.sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,
				sizeof(timeout)) < 0) {

		log_e("setsockopt failed\n");
		ret = -1;
	}

	if (setsockopt (client->info.sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,
				sizeof(timeout)) < 0) {

		log_e("setsockopt failed\n");
		ret = -1;
	}

	return ret;
}

static int __set_callback(struct tcp_client_t *client, int condition,
		CB_FUNCTION cb)
{
	struct info_t *info = &client->info;

	if (condition > MAX_CALLBACK_CONDITION || condition < 0) {
		return -1;
	}

	add_tcp_callback_list(info->callback_list[condition], cb);

	return 0;
}

static int __unset_callback(struct tcp_client_t *client, int condition, 
		CB_FUNCTION cb)
{
	struct info_t *info = &client->info;

	if (condition > MAX_CALLBACK_CONDITION || condition < 0) {
		return -1;
	}

	remove_tcp_callback_list(info->callback_list[condition], cb);

	return 0;
}

static int __run_locked(struct tcp_client_t *client)
{
	struct info_t *info = &client->info;
	int ret = 0;
	int err = 0;

	pthread_mutex_lock(&tcp_client_lock);

	if (info->is_run) {
		ret = -1;
	}

	err = pthread_create(&info->tid, 0, __run_thread, info);
	if (err != 0) {
		ret = -1;
	}

	info->is_run = true;
	info->stop = false;

	pthread_mutex_unlock(&tcp_client_lock);

	call_tcp_callback_list(info->callback_list[START_MONITOR], info);

	return 0;
}

static int __stop_locked(struct tcp_client_t *client)
{
	struct info_t *info = &client->info;
	int ret = 0;

	pthread_mutex_lock(&tcp_client_lock);

	info->stop = true;

	pthread_mutex_unlock(&tcp_client_lock);

	call_tcp_callback_list(info->callback_list[STOP_MONITOR], info);

	return ret;
}

static int __send_locked(struct info_t *info, const char *msg)
{
	int ret = 0;

	pthread_mutex_lock(&tcp_client_lock);

	strncpy(info->send_msg, msg, TCP_BUFFER);

	ret = send(info->sock, msg, strlen(msg), 0);

	pthread_mutex_unlock(&tcp_client_lock);

	if (ret < 0) {
		call_tcp_callback_list(info->callback_list[SEND_FAILED], info);
	}else {
		call_tcp_callback_list(info->callback_list[SEND_DATA], info);
	}

	return ret;
}

struct tcp_client_t* tcp_init_client()
{
	struct tcp_client_t *client;

	client = (struct tcp_client_t*)malloc(sizeof(struct tcp_client_t));
	if(client == NULL) {
		return NULL;
	}

	client->open = __open_socket;
	client->wait = __wait_socket;
	client->close = __close_socket;
	client->connect = __connect_socket;
	client->set_callback = __set_callback;
	client->unset_callback = __unset_callback;
	client->run = __run_locked;
	client->stop = __stop_locked;
	
	client->info.send = __send_locked;

	watchdog.info = &client->info;

	return client;
}

void tcp_terminate_client(struct tcp_client_t *client)
{
	if (client) {
		free(client);
	}

}
