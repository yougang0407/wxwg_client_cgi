#include <unistd.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event-internal.h>

#include <web_server.h>
#include <log.h>

#include "./nacd_interaction/include/nacd_tcp_client.h"


struct event_base *sg_current_base = NULL;

nacd_config_msg *nacd_cfg_ptr = NULL;
user_info *user_info_ptr = NULL;


int wxwgc_init_event(void)
{
	sg_current_base = event_init();
	if (NULL == sg_current_base) {
		return -1;
	}

	return 0;
}

int wxwgc_init_cfg(void)
{
	nacd_cfg_ptr = (nacd_config_msg *)malloc(sizeof(nacd_config_msg));
	if (NULL == nacd_cfg_ptr) {
		return -1;
	}

	user_info_ptr = (user_info *)malloc(sizeof(user_info));
	if (NULL == user_info_ptr) {
		return -1;
	}

	snprintf(nacd_cfg_ptr->nacd_server_ip, sizeof(nacd_cfg_ptr->nacd_server_ip), "0.0.0.0");
	nacd_cfg_ptr->nacd_server_port = 8859;
	nacd_cfg_ptr->timeout = 10;
	nacd_cfg_ptr->use_ssl = 0;

	return 0;
}

void Usage(const char* proc)
{
	WWC_DEBUG("%s [local_ip] [local_port]\n", proc);
}

int main(int argc, char *argv[])
{
	if (argc != 3) {
		Usage(argv[0]);
	}
	char local_ip[32] = {0,};
	int local_port = 8080;
	snprintf(local_ip, sizeof(local_ip), "%s", argv[1]);
	local_port = atoi(argv[2]);

	wxwgc_init_event();
	wxwgc_init_cfg();

	wxwgc_web_server_start(local_ip, local_port);

	//nacd_set_Heartbeat_timer();

	event_dispatch();

	free(user_info_ptr);
	free(nacd_cfg_ptr);
	return 0;
}
