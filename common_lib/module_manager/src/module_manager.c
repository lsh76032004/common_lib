#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include "module_manager.h"
#include "tcp_client.h"
#include "dl_parser.h"
#include "dl_loader.h"
#include "watchdog.h"
#include "logcat.h"
#include "live_update.h"

#define VERSION		"0.1.0"

struct settings_t settings;

void segv_handler(int sig)
{
	log_e("SIGFAULT!!!(%lu)\n", pthread_self());

	exit(1);
}

void sigpipe_handler(int sig)
{
	log_e("SIGPIPE!!!(%lu)\n", pthread_self());

	exit(1);
}

void help(char *argv)
{
	printf("[USAGE] %s [options]\n", argv);
	printf("	-p port [default: 33333]\n");
	printf("	-a address [default: 192.168.0.117]\n");
	printf("	-v print version info\n");
	printf("	-t run callbacks as threads\n");
	printf("	-w run watchdog\n");
	printf("	-l run logcat\n");
	printf("	-c print logcat at console(stdout)\n");
	printf("	-V print log verbose\n");
	printf("	-h help\n");

	exit(1);
}

int main(int argc, char *argv[])
{
	struct tcp_client_t *tcp_client;

	int c;

	char server_address[IPADDR_SIZE];
	int server_port;

	strncpy(server_address, "192.168.0.117", IPADDR_SIZE);
	server_port = 33333;

	while ((c = getopt (argc, argv, "p:a:vtwlcVh")) != -1) {
	switch (c)
	{
		case 'p':
			server_port = atoi(optarg);
			break;
		case 'a':
			strncpy(server_address, optarg, IPADDR_SIZE);
			break;
		case 'v':
			printf("VERSION: %s\n", VERSION);
			exit(1);
			break;
		case 't':
			settings.thread = true;
			break;
		case 'w':
			settings.watchdog = true;
			break;
		case 'l':
			settings.logcat = true;
			break;
		case 'c':
			settings.console = true;
			break;
		case 'V':
			settings.verbose = true;
			break;
		case 'h':
			/* Fall-Through */
		default:
			help(argv[0]);
			break;
	}
	}

	watchdog_init();
	logcat_init();

	log_d("Main tid=%lu\n", pthread_self());
	signal(SIGSEGV, segv_handler);
	signal(SIGPIPE, sigpipe_handler);

	watchdog_run();

	FILE *fp = dl_open_file();
	struct dl_info_t *dl_info = dl_read_info(fp);	
	dl_display_info(dl_info);

	tcp_client = tcp_init_client();
	if (tcp_client == NULL) {
		log_e("Failed to init tcp_client\n");
		exit(1);
	}

	tcp_client->open(tcp_client, server_address, server_port);
	tcp_client->connect(tcp_client);

	tcp_client->set_callback(tcp_client, SEND_DATA, watchdog_monitor_tcp_send_cb);
	tcp_client->set_callback(tcp_client, SEND_FAILED, watchdog_monitor_tcp_send_failed_cb);
	tcp_client->set_callback(tcp_client, RECV_DATA, watchdog_monitor_tcp_recv_cb);
	tcp_client->set_callback(tcp_client, RECV_DATA, live_update_check_cb);

	dl_load(dl_info);
	dl_register_callback(tcp_client, dl_info);

	tcp_client->run(tcp_client);
	tcp_client->wait(tcp_client);
	tcp_client->close(tcp_client);

	logcat_exit();

	return 0;
}
