#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>



struct event_base *sg_current_base = NULL;


int nac_init_event(void)
{
	sg_current_base = event_init();
	if(NULL == sg_current_base) {
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	nac_init_event();

	wxwgc_server_start();

	event_dispatch();

	return 0;
}
