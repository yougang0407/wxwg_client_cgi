#include <log.h>
#include <nacd_tcp_client.h>

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event-internal.h>


struct event_base *sg_current_base = NULL;


int test_nacd_init_event(void)
{
	sg_current_base = event_init();
	if (NULL == sg_current_base) {
		return -1;
	}

	return 0;
}



int main()
{
	test_nacd_init_event();

	user_info *user_msg = (user_info *)malloc(sizeof(user_info));
	nacd_config_msg *nacd_cfg = (nacd_config_msg *)malloc(sizeof(nacd_config_msg));;

	snprintf(user_msg->user_name, sizeof(user_msg->user_name), "admin");
	snprintf(user_msg->user_passwd, sizeof(user_msg->user_passwd), "123456");

	snprintf(nacd_cfg->nacd_server_ip, sizeof(nacd_cfg->nacd_server_ip), "0.0.0.0");
	nacd_cfg->nacd_server_port = 8859;
	nacd_cfg->timeout = 1;
	nacd_cfg->use_ssl = 0;

	int ret = nacd_handle_user_passwd_func(nacd_cfg, user_msg, NACD_USER_PASSWD_AUTH);
	get_user_attr_by_name(user_msg->user_name);
	event_dispatch();
	return ret;
}
