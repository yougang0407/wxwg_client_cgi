#include <unistd.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event-internal.h>

#include <web_server.h>
#include <log.h>


struct event_base *sg_current_base = NULL;


int wxwgc_init_event(void)
{
	sg_current_base = event_init();
	if (NULL == sg_current_base) {
		return -1;
	}

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
	char local_ip[32] = {"\0"};
	int local_port = 8080;
	snprintf(local_ip, sizeof(local_ip), "%s", argv[1]);
	local_port = atoi(argv[2]);

	wxwgc_init_event();
	wxwgc_web_server_start(local_ip, local_port);

	event_dispatch();
	return 0;
}
