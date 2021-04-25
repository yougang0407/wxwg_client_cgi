#ifndef __NACD_TCP_CLIENT_H__
#define __NACD_TCP_CLIENT_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <openssl/ssl.h>
#include <time.h>


#define INFO_SIZE			128
#define MAX_NAC_ID_LEN		64

#define _session_drop(X) \
do {				\
	if (X) {		\
		free(X);	\
		X = NULL;	\
	}				\
} while (0)

typedef unsigned int	uint32_t;

enum nacd_inter_protol_type {
	NACD_USER_PASSWD_AUTH,
	NACD_SEC_ASSERT_AAD,
	NACD_SEC_ASSERT_DEL,
	NACD_USER_PASSWD_AUTH_SUCCESS,
	NACD_USER_PASSWD_AUTH_FAILED,
	NACD_SEC_ASSERT_AAD_SUCCESS,
	NACD_SEC_ASSERT_AAD_FAILED,
	NACD_SEC_ASSERT_DEL_SUCCESS,
	NACD_SEC_ASSERT_DEL_FAILED,
	NACD_INTER_PROTOL_MAX,
};

enum nacd_state_e {
	NACD_CONNECT,
	NACD_SEND_USER_PASSWD,
	NACD_ADD_SEC_ASSERT,
	NACD_DEL_SEC_ASSERT,
	NACD_QUIT,
	NACD_STATE_MAX,
};

typedef struct nacdsock_t {
	int sock;
	SSL *ssl;
	SSL_CTX *ctx;
} nacdsock;

typedef struct nacd_config_msg_t {
	char nacd_server_ip[32];
	unsigned short nacd_server_port;
	unsigned short timeout;
	unsigned short use_ssl;
} nacd_config_msg;

typedef struct user_info_ptr_hdr_t {
	unsigned short type;
	unsigned short len;
} user_info_ptr_hdr;

typedef struct user_info_t {
	user_info_ptr_hdr user_info_hdr;
	char user_name[INFO_SIZE];
	char user_passwd[INFO_SIZE];
} user_info;

typedef struct sec_assert_info_t {
	user_info_ptr_hdr user_info_hdr;
	char auth_sec_assert_msg[2048];
} sec_assert_info;

typedef struct nacd_session_data_t {
	struct event ev;
	nacdsock *fd;
	struct sockaddr_in nacd_connec;
	int state;
	nacd_config_msg nacd_config_msg_data;
	user_info user_info_data;
	sec_assert_info sec_assert_info_data;
} nacd_session_data;

typedef struct nac_user_st {
	//struct list_head list;

	char from[32];
	char platform_id[32];
	char name[64];
	char id_card_num[32];
	char ethnic[8];
	char unit[64];
	char unit_name[64];
	char position[64];
	char gender[8];
	char identity_no[64];
	char soldier_security_no[64];
	char birthday[16];
	char contact_way[16];
	char blood_type[8];
	char rank[8];
	char militray_rank[8];
	char army_service[8];
	char nation[32];
	char security_card[32];
	char job[32];
	char login_name[64];
	char id_no[32];
	char register_time[32];
	char element[32];
	char sex[8];
	char unit_id[32];
	char has_fingerprint[8];
} nac_user;


int nacd_handle_user_passwd_func(nacd_config_msg *nacd_cfg_ptr, user_info *user_info_ptr, int nacd_state);
int nacd_handle_sec_assert_func(nacd_config_msg *nacd_cfg_ptr, char *auth_sec_assert_msg, int nacd_state);
nac_user *get_user_attr_by_name(char *username, char *g_websoap_user_url);
int get_random_func(char *ip, int port, char *res_random, int res_random_len);
void get_auth_sec_assert_msg_func(nac_user *user, char *res_random);

void nacd_set_Heartbeat_timer(void);


#endif
