/* soapClient.c
   Generated by gSOAP 2.8.71 for policy.h

gSOAP XML Web services tools
Copyright (C) 2000-2018, Robert van Engelen, Genivia Inc. All Rights Reserved.
The soapcpp2 tool and its generated software are released under the GPL.
This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------
A commercial use license is available from Genivia Inc., contact@genivia.com
--------------------------------------------------------------------------------
*/

#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.8.71 2020-11-26 17:02:15 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__getPermissionsByUserId(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_getPermissionsByUserIdReturn)
{	struct ns1__getPermissionsByUserId soap_tmp_ns1__getPermissionsByUserId;
	struct ns1__getPermissionsByUserIdResponse *soap_tmp_ns1__getPermissionsByUserIdResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.54:8086/app/services/AuthorizeService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__getPermissionsByUserId._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__getPermissionsByUserId(soap, &soap_tmp_ns1__getPermissionsByUserId);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__getPermissionsByUserId(soap, &soap_tmp_ns1__getPermissionsByUserId, "ns1:getPermissionsByUserId", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns1__getPermissionsByUserId(soap, &soap_tmp_ns1__getPermissionsByUserId, "ns1:getPermissionsByUserId", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_getPermissionsByUserIdReturn)
		return soap_closesock(soap);
	*_getPermissionsByUserIdReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__getPermissionsByUserIdResponse = soap_get_ns1__getPermissionsByUserIdResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__getPermissionsByUserIdResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_getPermissionsByUserIdReturn && soap_tmp_ns1__getPermissionsByUserIdResponse->_getPermissionsByUserIdReturn)
		*_getPermissionsByUserIdReturn = *soap_tmp_ns1__getPermissionsByUserIdResponse->_getPermissionsByUserIdReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__judgePermission(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_judgePermissionReturn)
{	struct ns1__judgePermission soap_tmp_ns1__judgePermission;
	struct ns1__judgePermissionResponse *soap_tmp_ns1__judgePermissionResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.54:8086/app/services/AuthorizeService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__judgePermission._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__judgePermission(soap, &soap_tmp_ns1__judgePermission);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__judgePermission(soap, &soap_tmp_ns1__judgePermission, "ns1:judgePermission", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns1__judgePermission(soap, &soap_tmp_ns1__judgePermission, "ns1:judgePermission", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_judgePermissionReturn)
		return soap_closesock(soap);
	*_judgePermissionReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__judgePermissionResponse = soap_get_ns1__judgePermissionResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__judgePermissionResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_judgePermissionReturn && soap_tmp_ns1__judgePermissionResponse->_judgePermissionReturn)
		*_judgePermissionReturn = *soap_tmp_ns1__judgePermissionResponse->_judgePermissionReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__getPermissionsByDeviceId(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_getPermissionsByDeviceIdReturn)
{	struct ns1__getPermissionsByDeviceId soap_tmp_ns1__getPermissionsByDeviceId;
	struct ns1__getPermissionsByDeviceIdResponse *soap_tmp_ns1__getPermissionsByDeviceIdResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.54:8086/app/services/AuthorizeService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__getPermissionsByDeviceId._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__getPermissionsByDeviceId(soap, &soap_tmp_ns1__getPermissionsByDeviceId);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__getPermissionsByDeviceId(soap, &soap_tmp_ns1__getPermissionsByDeviceId, "ns1:getPermissionsByDeviceId", "")
		 || soap_body_end_out(soap)
		 || soap_envelope_end_out(soap))
			 return soap->error;
	}
	if (soap_end_count(soap))
		return soap->error;
	if (soap_connect(soap, soap_endpoint, soap_action)
	 || soap_envelope_begin_out(soap)
	 || soap_putheader(soap)
	 || soap_body_begin_out(soap)
	 || soap_put_ns1__getPermissionsByDeviceId(soap, &soap_tmp_ns1__getPermissionsByDeviceId, "ns1:getPermissionsByDeviceId", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_getPermissionsByDeviceIdReturn)
		return soap_closesock(soap);
	*_getPermissionsByDeviceIdReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__getPermissionsByDeviceIdResponse = soap_get_ns1__getPermissionsByDeviceIdResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__getPermissionsByDeviceIdResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_getPermissionsByDeviceIdReturn && soap_tmp_ns1__getPermissionsByDeviceIdResponse->_getPermissionsByDeviceIdReturn)
		*_getPermissionsByDeviceIdReturn = *soap_tmp_ns1__getPermissionsByDeviceIdResponse->_getPermissionsByDeviceIdReturn;
	return soap_closesock(soap);
}

#ifdef __cplusplus
}
#endif

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapClient.c */
