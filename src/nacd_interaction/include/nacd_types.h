/******************************************************************************
 * 	Copyright (C), 2021-2021, legendsec Tech. Co., Ltd.
 *  File name: nacd_types.h
 *  Author: anonymity
 * 	Version:1.0
 * 	Date:2021-04-09
 * 	Description: This file defines all of the types common to the applications and modules.
 *
 * 	Others:
 * 		Function List:
 *
 *	History:
 * 		Date:2021-04-09
 * 		Author: anonymity
 * 		Modification: This file defines all of the types common to the applications and modules.
 *
 *******************************************************************************/

#ifndef __NACD_TYPES_H__
#define __NACD_TYPES_H__


/* error code */
#define NACD_SUCCESS 				0		/* Successful function return */
#define NACD_MODULE_NULL			1		/* Module is NULL */
#define NACD_MODULE_ERR				2		/* Module not found */
#define NACD_SERVICE_ERR 			3		/* Error in service module */
#define NACD_SYSTEM_ERR 			4		/* System error */
#define NACD_BUF_ERR 				5		/* Memory buffer error */
#define NACD_PERM_DENIED 			6		/* Permission denied */
#define NACD_AUTH_ERR 				7		/* Authentication failure */
#define NACD_ACCOUNT_ERR 			8		/* Account not found */
#define NACD_ACCOUNT_FULL 			9		/* Account is over */
#define NACD_ABORT 					10
#define NACD_NO_MODULE_DATA			11		/* No module specific data is present */
#define NACD_BAD_ITEM				12		/* Bad item passed to nacd_*_item() */
#define NACD_INCOMPLETE				13
#define NACD_AUTHTOK_ERR 			14		/* Authentication token manipulation error */
#define NACD_DATA_ERR 				15		/* Nacd data error */
#define NACD_USER_UNKNOWN 			16		/* User not known to the authsrv */
#define NACD_USER_EXPIRE 			17		/* User expire  */
#define NACD_CONN_TIMEOUT 			18		/* Connect to server timeout */
#define NACD_BINDDN_ERR 			19		/* Bind binddn failure */
#define NACD_SEARCH_METH_ERR 		20		/* Bind binddn failure */
#define NACD_CONNECTION_SERVER		21
#define NACD_SYNC_ERROR				22
#define NACD_SYN_NUM_LIMIT			23
#define NACD_PASSWORD_ERROR			24
#define NACD_SYNCING				25
#define NACD_HAVE_SERVER_SYNCING	26
#define NACD_BASEDN_ERR				27
#define NACD_TESTING				28
#define NACD_HAVE_SERVER_TESTING	29
#define NACD_LOCALDB_ERR 			30		/* db ERROR */


#endif

