/* soapClient.c
   Generated by gSOAP 2.8.71 for res.h

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

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.8.71 2020-11-26 17:53:12 GMT")


SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__addDeviceBatch(struct soap *soap, const char *soap_endpoint, const char *soap_action, char **addDeviceBatchReturn)
{	struct ns1__addDeviceBatch soap_tmp_ns1__addDeviceBatch;
	struct ns1__addDeviceBatchResponse *soap_tmp_ns1__addDeviceBatchResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__addDeviceBatch(soap, &soap_tmp_ns1__addDeviceBatch);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__addDeviceBatch(soap, &soap_tmp_ns1__addDeviceBatch, "ns1:addDeviceBatch", "")
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
	 || soap_put_ns1__addDeviceBatch(soap, &soap_tmp_ns1__addDeviceBatch, "ns1:addDeviceBatch", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!addDeviceBatchReturn)
		return soap_closesock(soap);
	*addDeviceBatchReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__addDeviceBatchResponse = soap_get_ns1__addDeviceBatchResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__addDeviceBatchResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (addDeviceBatchReturn && soap_tmp_ns1__addDeviceBatchResponse->addDeviceBatchReturn)
		*addDeviceBatchReturn = *soap_tmp_ns1__addDeviceBatchResponse->addDeviceBatchReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DeviceDefine(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DeviceDefineReturn)
{	struct ns1__DeviceDefine soap_tmp_ns1__DeviceDefine;
	struct ns1__DeviceDefineResponse *soap_tmp_ns1__DeviceDefineResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DeviceDefine._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DeviceDefine(soap, &soap_tmp_ns1__DeviceDefine);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DeviceDefine(soap, &soap_tmp_ns1__DeviceDefine, "ns1:DeviceDefine", "")
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
	 || soap_put_ns1__DeviceDefine(soap, &soap_tmp_ns1__DeviceDefine, "ns1:DeviceDefine", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DeviceDefineReturn)
		return soap_closesock(soap);
	*_DeviceDefineReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DeviceDefineResponse = soap_get_ns1__DeviceDefineResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DeviceDefineResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DeviceDefineReturn && soap_tmp_ns1__DeviceDefineResponse->_DeviceDefineReturn)
		*_DeviceDefineReturn = *soap_tmp_ns1__DeviceDefineResponse->_DeviceDefineReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__SoftListGet(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_SoftListGetReturn)
{	struct ns1__SoftListGet soap_tmp_ns1__SoftListGet;
	struct ns1__SoftListGetResponse *soap_tmp_ns1__SoftListGetResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__SoftListGet._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__SoftListGet(soap, &soap_tmp_ns1__SoftListGet);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__SoftListGet(soap, &soap_tmp_ns1__SoftListGet, "ns1:SoftListGet", "")
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
	 || soap_put_ns1__SoftListGet(soap, &soap_tmp_ns1__SoftListGet, "ns1:SoftListGet", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_SoftListGetReturn)
		return soap_closesock(soap);
	*_SoftListGetReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__SoftListGetResponse = soap_get_ns1__SoftListGetResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__SoftListGetResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_SoftListGetReturn && soap_tmp_ns1__SoftListGetResponse->_SoftListGetReturn)
		*_SoftListGetReturn = *soap_tmp_ns1__SoftListGetResponse->_SoftListGetReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__SoftDefine(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_SoftDefineReturn)
{	struct ns1__SoftDefine soap_tmp_ns1__SoftDefine;
	struct ns1__SoftDefineResponse *soap_tmp_ns1__SoftDefineResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__SoftDefine._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__SoftDefine(soap, &soap_tmp_ns1__SoftDefine);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__SoftDefine(soap, &soap_tmp_ns1__SoftDefine, "ns1:SoftDefine", "")
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
	 || soap_put_ns1__SoftDefine(soap, &soap_tmp_ns1__SoftDefine, "ns1:SoftDefine", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_SoftDefineReturn)
		return soap_closesock(soap);
	*_SoftDefineReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__SoftDefineResponse = soap_get_ns1__SoftDefineResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__SoftDefineResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_SoftDefineReturn && soap_tmp_ns1__SoftDefineResponse->_SoftDefineReturn)
		*_SoftDefineReturn = *soap_tmp_ns1__SoftDefineResponse->_SoftDefineReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__Challenge(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_ChallengeReturn)
{	struct ns1__Challenge soap_tmp_ns1__Challenge;
	struct ns1__ChallengeResponse *soap_tmp_ns1__ChallengeResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__Challenge._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__Challenge(soap, &soap_tmp_ns1__Challenge);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__Challenge(soap, &soap_tmp_ns1__Challenge, "ns1:Challenge", "")
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
	 || soap_put_ns1__Challenge(soap, &soap_tmp_ns1__Challenge, "ns1:Challenge", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_ChallengeReturn)
		return soap_closesock(soap);
	*_ChallengeReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__ChallengeResponse = soap_get_ns1__ChallengeResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__ChallengeResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_ChallengeReturn && soap_tmp_ns1__ChallengeResponse->_ChallengeReturn)
		*_ChallengeReturn = *soap_tmp_ns1__ChallengeResponse->_ChallengeReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DataListGet(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DataListGetReturn)
{	struct ns1__DataListGet soap_tmp_ns1__DataListGet;
	struct ns1__DataListGetResponse *soap_tmp_ns1__DataListGetResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DataListGet._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DataListGet(soap, &soap_tmp_ns1__DataListGet);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DataListGet(soap, &soap_tmp_ns1__DataListGet, "ns1:DataListGet", "")
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
	 || soap_put_ns1__DataListGet(soap, &soap_tmp_ns1__DataListGet, "ns1:DataListGet", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DataListGetReturn)
		return soap_closesock(soap);
	*_DataListGetReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DataListGetResponse = soap_get_ns1__DataListGetResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DataListGetResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DataListGetReturn && soap_tmp_ns1__DataListGetResponse->_DataListGetReturn)
		*_DataListGetReturn = *soap_tmp_ns1__DataListGetResponse->_DataListGetReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DataDefine(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DataDefineReturn)
{	struct ns1__DataDefine soap_tmp_ns1__DataDefine;
	struct ns1__DataDefineResponse *soap_tmp_ns1__DataDefineResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DataDefine._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DataDefine(soap, &soap_tmp_ns1__DataDefine);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DataDefine(soap, &soap_tmp_ns1__DataDefine, "ns1:DataDefine", "")
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
	 || soap_put_ns1__DataDefine(soap, &soap_tmp_ns1__DataDefine, "ns1:DataDefine", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DataDefineReturn)
		return soap_closesock(soap);
	*_DataDefineReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DataDefineResponse = soap_get_ns1__DataDefineResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DataDefineResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DataDefineReturn && soap_tmp_ns1__DataDefineResponse->_DataDefineReturn)
		*_DataDefineReturn = *soap_tmp_ns1__DataDefineResponse->_DataDefineReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DeviceCancel(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DeviceCancelReturn)
{	struct ns1__DeviceCancel soap_tmp_ns1__DeviceCancel;
	struct ns1__DeviceCancelResponse *soap_tmp_ns1__DeviceCancelResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DeviceCancel._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DeviceCancel(soap, &soap_tmp_ns1__DeviceCancel);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DeviceCancel(soap, &soap_tmp_ns1__DeviceCancel, "ns1:DeviceCancel", "")
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
	 || soap_put_ns1__DeviceCancel(soap, &soap_tmp_ns1__DeviceCancel, "ns1:DeviceCancel", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DeviceCancelReturn)
		return soap_closesock(soap);
	*_DeviceCancelReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DeviceCancelResponse = soap_get_ns1__DeviceCancelResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DeviceCancelResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DeviceCancelReturn && soap_tmp_ns1__DeviceCancelResponse->_DeviceCancelReturn)
		*_DeviceCancelReturn = *soap_tmp_ns1__DeviceCancelResponse->_DeviceCancelReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DeviceModify(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DeviceModifyReturn)
{	struct ns1__DeviceModify soap_tmp_ns1__DeviceModify;
	struct ns1__DeviceModifyResponse *soap_tmp_ns1__DeviceModifyResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DeviceModify._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DeviceModify(soap, &soap_tmp_ns1__DeviceModify);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DeviceModify(soap, &soap_tmp_ns1__DeviceModify, "ns1:DeviceModify", "")
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
	 || soap_put_ns1__DeviceModify(soap, &soap_tmp_ns1__DeviceModify, "ns1:DeviceModify", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DeviceModifyReturn)
		return soap_closesock(soap);
	*_DeviceModifyReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DeviceModifyResponse = soap_get_ns1__DeviceModifyResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DeviceModifyResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DeviceModifyReturn && soap_tmp_ns1__DeviceModifyResponse->_DeviceModifyReturn)
		*_DeviceModifyReturn = *soap_tmp_ns1__DeviceModifyResponse->_DeviceModifyReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DeviceListGet(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DeviceListGetReturn)
{	struct ns1__DeviceListGet soap_tmp_ns1__DeviceListGet;
	struct ns1__DeviceListGetResponse *soap_tmp_ns1__DeviceListGetResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DeviceListGet._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DeviceListGet(soap, &soap_tmp_ns1__DeviceListGet);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DeviceListGet(soap, &soap_tmp_ns1__DeviceListGet, "ns1:DeviceListGet", "")
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
	 || soap_put_ns1__DeviceListGet(soap, &soap_tmp_ns1__DeviceListGet, "ns1:DeviceListGet", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DeviceListGetReturn)
		return soap_closesock(soap);
	*_DeviceListGetReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DeviceListGetResponse = soap_get_ns1__DeviceListGetResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DeviceListGetResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DeviceListGetReturn && soap_tmp_ns1__DeviceListGetResponse->_DeviceListGetReturn)
		*_DeviceListGetReturn = *soap_tmp_ns1__DeviceListGetResponse->_DeviceListGetReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__SoftCancel(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_SoftCancelReturn)
{	struct ns1__SoftCancel soap_tmp_ns1__SoftCancel;
	struct ns1__SoftCancelResponse *soap_tmp_ns1__SoftCancelResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__SoftCancel._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__SoftCancel(soap, &soap_tmp_ns1__SoftCancel);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__SoftCancel(soap, &soap_tmp_ns1__SoftCancel, "ns1:SoftCancel", "")
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
	 || soap_put_ns1__SoftCancel(soap, &soap_tmp_ns1__SoftCancel, "ns1:SoftCancel", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_SoftCancelReturn)
		return soap_closesock(soap);
	*_SoftCancelReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__SoftCancelResponse = soap_get_ns1__SoftCancelResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__SoftCancelResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_SoftCancelReturn && soap_tmp_ns1__SoftCancelResponse->_SoftCancelReturn)
		*_SoftCancelReturn = *soap_tmp_ns1__SoftCancelResponse->_SoftCancelReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__SoftModify(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_SoftModifyReturn)
{	struct ns1__SoftModify soap_tmp_ns1__SoftModify;
	struct ns1__SoftModifyResponse *soap_tmp_ns1__SoftModifyResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__SoftModify._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__SoftModify(soap, &soap_tmp_ns1__SoftModify);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__SoftModify(soap, &soap_tmp_ns1__SoftModify, "ns1:SoftModify", "")
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
	 || soap_put_ns1__SoftModify(soap, &soap_tmp_ns1__SoftModify, "ns1:SoftModify", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_SoftModifyReturn)
		return soap_closesock(soap);
	*_SoftModifyReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__SoftModifyResponse = soap_get_ns1__SoftModifyResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__SoftModifyResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_SoftModifyReturn && soap_tmp_ns1__SoftModifyResponse->_SoftModifyReturn)
		*_SoftModifyReturn = *soap_tmp_ns1__SoftModifyResponse->_SoftModifyReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DataCancel(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DataCancelReturn)
{	struct ns1__DataCancel soap_tmp_ns1__DataCancel;
	struct ns1__DataCancelResponse *soap_tmp_ns1__DataCancelResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DataCancel._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DataCancel(soap, &soap_tmp_ns1__DataCancel);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DataCancel(soap, &soap_tmp_ns1__DataCancel, "ns1:DataCancel", "")
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
	 || soap_put_ns1__DataCancel(soap, &soap_tmp_ns1__DataCancel, "ns1:DataCancel", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DataCancelReturn)
		return soap_closesock(soap);
	*_DataCancelReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DataCancelResponse = soap_get_ns1__DataCancelResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DataCancelResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DataCancelReturn && soap_tmp_ns1__DataCancelResponse->_DataCancelReturn)
		*_DataCancelReturn = *soap_tmp_ns1__DataCancelResponse->_DataCancelReturn;
	return soap_closesock(soap);
}

SOAP_FMAC5 int SOAP_FMAC6 soap_call_ns1__DataModify(struct soap *soap, const char *soap_endpoint, const char *soap_action, char *_inStr, char **_DataModifyReturn)
{	struct ns1__DataModify soap_tmp_ns1__DataModify;
	struct ns1__DataModifyResponse *soap_tmp_ns1__DataModifyResponse;
	if (soap_endpoint == NULL)
		soap_endpoint = "http://172.30.202.41:8086/app/services/RmsWebService";
	if (soap_action == NULL)
		soap_action = "";
	soap_tmp_ns1__DataModify._inStr = _inStr;
	soap_begin(soap);
	soap->encodingStyle = "http://schemas.xmlsoap.org/soap/encoding/";
	soap_serializeheader(soap);
	soap_serialize_ns1__DataModify(soap, &soap_tmp_ns1__DataModify);
	if (soap_begin_count(soap))
		return soap->error;
	if (soap->mode & SOAP_IO_LENGTH)
	{	if (soap_envelope_begin_out(soap)
		 || soap_putheader(soap)
		 || soap_body_begin_out(soap)
		 || soap_put_ns1__DataModify(soap, &soap_tmp_ns1__DataModify, "ns1:DataModify", "")
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
	 || soap_put_ns1__DataModify(soap, &soap_tmp_ns1__DataModify, "ns1:DataModify", "")
	 || soap_body_end_out(soap)
	 || soap_envelope_end_out(soap)
	 || soap_end_send(soap))
		return soap_closesock(soap);
	if (!_DataModifyReturn)
		return soap_closesock(soap);
	*_DataModifyReturn = NULL;
	if (soap_begin_recv(soap)
	 || soap_envelope_begin_in(soap)
	 || soap_recv_header(soap)
	 || soap_body_begin_in(soap))
		return soap_closesock(soap);
	if (soap_recv_fault(soap, 1))
		return soap->error;
	soap_tmp_ns1__DataModifyResponse = soap_get_ns1__DataModifyResponse(soap, NULL, "", NULL);
	if (!soap_tmp_ns1__DataModifyResponse || soap->error)
		return soap_recv_fault(soap, 0);
	if (soap_body_end_in(soap)
	 || soap_envelope_end_in(soap)
	 || soap_end_recv(soap))
		return soap_closesock(soap);
	if (_DataModifyReturn && soap_tmp_ns1__DataModifyResponse->_DataModifyReturn)
		*_DataModifyReturn = *soap_tmp_ns1__DataModifyResponse->_DataModifyReturn;
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
