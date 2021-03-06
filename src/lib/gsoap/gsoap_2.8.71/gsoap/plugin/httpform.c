/*
        httpform.c

        gSOAP HTTP POST application/x-www-form-urlencoded data plugin.

        Requires linkage with httpget.c (for soap_query_key and soap_query_val)

        Note: multipart/related and multipart/form-data are handled in gSOAP as
        MIME attachments.

gSOAP XML Web services tools
Copyright (C) 2000-2008, Robert van Engelen, Genivia Inc., All Rights Reserved.
This part of the software is released under ONE of the following licenses:
GPL, the gSOAP public license, OR Genivia's license for commercial use.
--------------------------------------------------------------------------------
gSOAP public license.

The contents of this file are subject to the gSOAP Public License Version 1.3
(the "License"); you may not use this file except in compliance with the
License. You may obtain a copy of the License at
http://www.cs.fsu.edu/~engelen/soaplicense.html
Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the License.

The Initial Developer of the Original Code is Robert A. van Engelen.
Copyright (C) 2000-2004 Robert A. van Engelen, Genivia inc. All Rights Reserved.
--------------------------------------------------------------------------------
GPL license.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA

Author contact information:
engelen@genivia.com / engelen@acm.org

This program is released under the GPL with the additional exemption that
compiling, linking, and/or using OpenSSL is allowed.
--------------------------------------------------------------------------------

        Compile & link with stand-alone gSOAP server.

        Usage (server side):
        struct soap soap;
        soap_init(&soap);
        soap_register_plugin_arg(&soap, http_form, http_form_handler);
        ...
        ... = soap_copy(&soap); // copies plugin too but not its data: plugin data is shared since fcopy is not set
        ...
        soap_done(&soap); // detach plugin (calls plugin->fdelete)

        You need to define a HTTP handling function at the server-side:
        int http_form_handler(struct soap*)
        which will be called from the plugin upon an HTTP POST request that
        matches the application/x-www-form-urlencoded content-type.
        The function should return an error code or SOAP_STOP to prevent the
        gSOAP engine from processing the message body;

        To parse form data in the handler, use:

        char *s = soap_get_form(soap);
        while (s)
        {
          char *key = soap_query_key(soap, &s); // decode next form string key
          char *val = soap_query_val(soap, &s); // decode next form string value (if any)
          ...
        }

        The soap_get_form() function reads an HTTP body and stores it in an
        internal buffer that is returned as a char*. This buffer can be used to
        process HTTP POST body content. The soap_query_key/val functions simply
        extract key-value pairs from this buffer.

        The handler should also produce a valid HTTP response, for example:
        soap_response(soap, SOAP_HTML); // use this to return HTML ...
        soap_response(soap, SOAP_OK); // ... or use this to return a SOAP message
        ...
        soap_send(soap, "<HTML>...</HTML>"); // example HTML
        ...
        soap_end_send(soap);

        See samples/webserver for an example HTTP POST form handling server.

        Warning: this plugin MUST be registered AFTER the httppost plugin.
*/

#include "httpform.h"

#ifdef __cplusplus
extern "C" {
#endif

const char http_form_id[] = HTTP_FORM_ID;

static int http_form_init(struct soap *soap, struct http_form_data *data, int (*handler)(struct soap*));
static void http_form_delete(struct soap *soap, struct soap_plugin *p);
static int http_form_parse_header(struct soap *soap, const char*, const char*);

int http_form(struct soap *soap, struct soap_plugin *p, void *arg)
{
  p->id = http_form_id;
  p->data = (void*)malloc(sizeof(struct http_form_data));
  p->fdelete = http_form_delete;
  if (!p->data)
    return SOAP_EOM;
  if (http_form_init(soap, (struct http_form_data*)p->data, (int (*)(struct soap*))arg))
  {
    free(p->data); /* error: could not init */
    return SOAP_EOM; /* return error */
  }
  return SOAP_OK;
}

static int http_form_init(struct soap *soap, struct http_form_data *data, int (*handler)(struct soap*))
{
  data->fparsehdr = soap->fparsehdr; /* save old HTTP header parser callback */
  soap->fparsehdr = http_form_parse_header; /* replace HTTP header parser callback with ours */
  if (handler)
    data->handler = handler;
  return SOAP_OK;
}

static void http_form_delete(struct soap *soap, struct soap_plugin *p)
{
  (void)soap;
  free(p->data); /* free allocated plugin data (this function is not called for shared plugin data, but only when the final soap_done() is invoked on the original soap struct) */
}

static int http_form_parse_header(struct soap *soap, const char *key, const char *val)
{
  struct http_form_data *data = (struct http_form_data*)soap_lookup_plugin(soap, http_form_id);
  if (!data)
    return SOAP_PLUGIN_ERROR;
  if (!soap_tag_cmp(key, "Content-Type"))
  {
    /* check content type: you can filter any type of payloads here */
    if (!soap_tag_cmp(val, "application/x-www-form-urlencoded")
     || !soap_tag_cmp(val, "application/x-www-form-urlencoded;*")) /* skip charset=UTF-8 etc */
    {
      soap->fform = data->handler;
      return soap->error = SOAP_FORM; /* delegate body parsing to form handler */
    }
    /* it is possible to add other payload types to handle via forms and use * as a wildcard:
       if (!soap_tag_cmp(val, "image/jpg") || !soap_tag_cmp(val, "image/jpg;*"))
       soap->error = SOAP_FORM;
     */
  }
  return data->fparsehdr(soap, key, val); /* parse HTTP header */
}

char * soap_get_form(struct soap *soap)
{
  char *s;
  ULONG64 k = soap->length;
  /* check HTTP body, return "" if none */
  if (!k && !(soap->mode & SOAP_ENC_ZLIB) && (soap->mode & SOAP_IO) != SOAP_IO_CHUNK)
    return soap_strdup(soap, "?");
  /* do not consume DIME or MIME attachments */
  if ((soap->mode & SOAP_ENC_DIME))
  {
    soap->error = SOAP_DIME_ERROR;
    return NULL;
  }
  if ((soap->mode & SOAP_ENC_MIME))
  {
    soap->error = SOAP_MIME_ERROR;
    return NULL;
  }
  DBGLOG(TEST, SOAP_MESSAGE(fdebug, "Parsing HTTP body (mode=0x%x)\n", soap->mode));
  if (k && !(soap->mode & SOAP_ENC_ZLIB))
  {
    char *t;
    /* http content length != 0 and uncompressed body */
    if ((SOAP_MAXALLOCSIZE != 0 && k > SOAP_MAXALLOCSIZE) || k > (ULONG64)((size_t)-3))
    {
      soap->error = SOAP_EOM;
      return NULL;
    }
    s = t = (char*)soap_malloc(soap, (size_t)k + 2);
    if (s)
    {
      size_t i;
      *t++ = '?';
      for (i = 0; i < k; i++)
      {
        soap_wchar c = soap_get1(soap);
        if ((int)c == EOF)
          break;
        *t++ = (char)(c & 0xFF);
      }
      *t = '\0';
    }
    else
    {
      soap->error = SOAP_EOM;
      return NULL;
    }
  }
  else
  {
    size_t i, l = 0;
    if (soap_alloc_block(soap) == NULL)
      return NULL;
    s = (char*)soap_push_block(soap, NULL, 1);
    if (!s)
      return NULL;
    *s = '?';
    for (;;)
    {
      size_t k = SOAP_BLKLEN;
      s = (char*)soap_push_block(soap, NULL, k);
      if (!s)
        return NULL;
      for (i = 0; i < k; i++)
      {
        soap_wchar c;
        l++;
        if (l == 0)
        {
          soap->error = SOAP_EOM;
          return NULL;
        }
        c = soap_get1(soap);
        if ((int)c == EOF)
          goto end;
        *s++ = (char)(c & 0xFF);
      }
    }
end:
    *s = '\0';
    soap_size_block(soap, NULL, i + 1);
    s = soap_save_block(soap, NULL, NULL, 0);
  }
  return s;
}
  
/******************************************************************************/

#ifdef __cplusplus
}
#endif

