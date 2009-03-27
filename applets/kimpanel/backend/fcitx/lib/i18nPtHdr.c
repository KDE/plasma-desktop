/******************************************************************
 
         Copyright 1994, 1995 by Sun Microsystems, Inc.
         Copyright 1993, 1994 by Hewlett-Packard Company
 
Permission to use, copy, modify, distribute, and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation, and that the name of Sun Microsystems, Inc.
and Hewlett-Packard not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Sun Microsystems, Inc. and Hewlett-Packard make no representations about
the suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
 
SUN MICROSYSTEMS INC. AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
SUN MICROSYSTEMS, INC. AND HEWLETT-PACKARD COMPANY BE LIABLE FOR ANY
SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
  Author: Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.

    This version tidied and debugged by Steve Underwood May 1999
 
******************************************************************/

#include <stdlib.h>
#include <sys/param.h>
#include <X11/Xlib.h>
#ifndef NEED_EVENTS
#define NEED_EVENTS
#endif
#include <X11/Xproto.h>
#undef NEED_EVENTS
#include "FrameMgr.h"
#include "IMdkit.h"
#include "Xi18n.h"
#include "XimFunc.h"

#ifdef XIM_DEBUG
#include <stdio.h>

static void	DebugLog(char * msg)
{
	fprintf(stderr, msg);
}
#endif

extern Xi18nClient *_Xi18nFindClient (Xi18n, CARD16);

static void GetProtocolVersion (CARD16 client_major,
                                CARD16 client_minor,
                                CARD16 *server_major,
                                CARD16 *server_minor)
{
    *server_major = client_major;
    *server_minor = client_minor;
}

static void ConnectMessageProc (XIMS ims,
                                IMProtocol *call_data,
                                unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec connect_fr[], connect_reply_fr[];
    register int total_size;
    CARD16 server_major_version, server_minor_version;
    unsigned char *reply = NULL;
    IMConnectStruct *imconnect =
        (IMConnectStruct*) &call_data->imconnect;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (connect_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* get data */
    FrameMgrGetToken (fm, imconnect->byte_order);
    FrameMgrGetToken (fm, imconnect->major_version);
    FrameMgrGetToken (fm, imconnect->minor_version);

    FrameMgrFree (fm);

    GetProtocolVersion (imconnect->major_version,
                        imconnect->minor_version,
                        &server_major_version,
                        &server_minor_version);
#ifdef PROTOCOL_RICH
    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
#endif  /* PROTOCOL_RICH */

    fm = FrameMgrInit (connect_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, server_major_version);
    FrameMgrPutToken (fm, server_minor_version);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_CONNECT_REPLY,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree (reply);
}

static void DisConnectMessageProc (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    unsigned char *reply = NULL;
    CARD16 connect_id = call_data->any.connect_id;

#ifdef PROTOCOL_RICH
    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
#endif  /* PROTOCOL_RICH */

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_DISCONNECT_REPLY,
                       0,
                       reply,
                       0);

    i18n_core->methods.disconnect (ims, connect_id);
}

static void OpenMessageProc(XIMS ims, IMProtocol *call_data, unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec open_fr[];
    extern XimFrameRec open_reply_fr[];
    unsigned char *reply = NULL;
    int str_size;
    register int i, total_size;
    CARD16 connect_id = call_data->any.connect_id;
    int str_length;
    char *name;
    IMOpenStruct *imopen = (IMOpenStruct *) &call_data->imopen;

    fm = FrameMgrInit (open_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* get data */
    FrameMgrGetToken (fm, str_length);
    FrameMgrSetSize (fm, str_length);
    FrameMgrGetToken (fm, name);
    imopen->lang.length = str_length;
    imopen->lang.name = malloc (str_length + 1);
    strncpy (imopen->lang.name, name, str_length);
    imopen->lang.name[str_length] = (char) 0;

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
    if ((i18n_core->address.imvalue_mask & I18N_ON_KEYS)
        ||
        (i18n_core->address.imvalue_mask & I18N_OFF_KEYS))
    {
        _Xi18nSendTriggerKey (ims, connect_id);
    }
    /*endif*/
    XFree (imopen->lang.name);

    fm = FrameMgrInit (open_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* set iteration count for list of imattr */
    FrameMgrSetIterCount (fm, i18n_core->address.im_attr_num);

    /* set length of BARRAY item in ximattr_fr */
    for (i = 0;  i < i18n_core->address.im_attr_num;  i++)
    {
        str_size = strlen (i18n_core->address.xim_attr[i].name);
        FrameMgrSetSize (fm, str_size);
    }
    /*endfor*/
    /* set iteration count for list of icattr */
    FrameMgrSetIterCount (fm, i18n_core->address.ic_attr_num);
    /* set length of BARRAY item in xicattr_fr */
    for (i = 0;  i < i18n_core->address.ic_attr_num;  i++)
    {
        str_size = strlen (i18n_core->address.xic_attr[i].name);
        FrameMgrSetSize (fm, str_size);
    }
    /*endfor*/

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    /* input input-method ID */
    FrameMgrPutToken (fm, connect_id);

    for (i = 0;  i < i18n_core->address.im_attr_num;  i++)
    {
        str_size = FrameMgrGetSize (fm);
        FrameMgrPutToken (fm, i18n_core->address.xim_attr[i].attribute_id);
        FrameMgrPutToken (fm, i18n_core->address.xim_attr[i].type);
        FrameMgrPutToken (fm, str_size);
        FrameMgrPutToken (fm, i18n_core->address.xim_attr[i].name);
    }
    /*endfor*/
    for (i = 0;  i < i18n_core->address.ic_attr_num;  i++)
    {
        str_size = FrameMgrGetSize (fm);
        FrameMgrPutToken (fm, i18n_core->address.xic_attr[i].attribute_id);
        FrameMgrPutToken (fm, i18n_core->address.xic_attr[i].type);
        FrameMgrPutToken (fm, str_size);
        FrameMgrPutToken (fm, i18n_core->address.xic_attr[i].name);
    }
    /*endfor*/

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_OPEN_REPLY,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree (reply);
}

static void CloseMessageProc (XIMS ims,
                              IMProtocol *call_data,
                              unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec close_fr[];
    extern XimFrameRec close_reply_fr[];
    unsigned char *reply = NULL;
    register int total_size;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (close_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    FrameMgrGetToken (fm, input_method_ID);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/

    fm = FrameMgrInit (close_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims,
                           connect_id,
                           XIM_ERROR,
                           0,
                           0,
                           0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_CLOSE_REPLY,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree (reply);
}

static XIMExt *MakeExtensionList (Xi18n i18n_core,
                                  XIMStr *lib_extension,
                                  int number,
                                  int *reply_number)
{
    XIMExt *ext_list;
    XIMExt *im_ext = (XIMExt *) i18n_core->address.extension;
    int im_ext_len = i18n_core->address.ext_num;
    int i;
    int j;

    *reply_number = 0;

    if (number == 0)
    {
        /* query all extensions */
        *reply_number = im_ext_len;
    }
    else
    {
        for (i = 0;  i < im_ext_len;  i++)
        {
            for (j = 0;  j < (int) number;  j++)
            {
                if (strcmp (lib_extension[j].name, im_ext[i].name) == 0)
                {
                    (*reply_number)++;
                    break;
                }
                /*endif*/
            }
            /*endfor*/
        }
        /*endfor*/
    }
    /*endif*/

    if (!(*reply_number))
        return NULL;
    /*endif*/
    ext_list = (XIMExt *) malloc (sizeof (XIMExt)*(*reply_number));
    if (!ext_list)
        return NULL;
    /*endif*/
    memset (ext_list, 0, sizeof (XIMExt)*(*reply_number));

    if (number == 0)
    {
        /* query all extensions */
        for (i = 0;  i < im_ext_len;  i++)
        {
            ext_list[i].major_opcode = im_ext[i].major_opcode;
            ext_list[i].minor_opcode = im_ext[i].minor_opcode;
            ext_list[i].length = im_ext[i].length;
            ext_list[i].name = malloc (im_ext[i].length + 1);
            strcpy (ext_list[i].name, im_ext[i].name);
        }
        /*endfor*/
    }
    else
    {
        int n = 0;

        for (i = 0;  i < im_ext_len;  i++)
        {
            for (j = 0;  j < (int)number;  j++)
            {
                if (strcmp (lib_extension[j].name, im_ext[i].name) == 0)
                {
                    ext_list[n].major_opcode = im_ext[i].major_opcode;
                    ext_list[n].minor_opcode = im_ext[i].minor_opcode;
                    ext_list[n].length = im_ext[i].length;
                    ext_list[n].name = malloc (im_ext[i].length + 1);
                    strcpy (ext_list[n].name, im_ext[i].name);
                    n++;
                    break;
                }
                /*endif*/
            }
            /*endfor*/
        }
        /*endfor*/
    }
    /*endif*/
    return ext_list;
}

static void QueryExtensionMessageProc (XIMS ims,
                                       IMProtocol *call_data,
                                       unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    FmStatus status;
    extern XimFrameRec query_extension_fr[];
    extern XimFrameRec query_extension_reply_fr[];
    unsigned char *reply = NULL;
    int str_size;
    register int i;
    register int number;
    register int total_size;
    int byte_length;
    int reply_number = 0;
    XIMExt *ext_list;
    IMQueryExtensionStruct *query_ext =
        (IMQueryExtensionStruct *) &call_data->queryext;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (query_extension_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, byte_length);
    query_ext->extension = (XIMStr *) malloc (sizeof (XIMStr)*10);
    memset (query_ext->extension, 0, sizeof (XIMStr)*10);
    number = 0;
    while (FrameMgrIsIterLoopEnd (fm, &status) == False)
    {
        char *name;
        int str_length;
        
        FrameMgrGetToken (fm, str_length);
        FrameMgrSetSize (fm, str_length);
        query_ext->extension[number].length = str_length;
        FrameMgrGetToken (fm, name);
        query_ext->extension[number].name = malloc (str_length + 1);
        strncpy (query_ext->extension[number].name, name, str_length);
        query_ext->extension[number].name[str_length] = (char) 0;
        number++;
    }
    /*endwhile*/
    query_ext->number = number;

#ifdef PROTOCOL_RICH
    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
#endif  /* PROTOCOL_RICH */

    FrameMgrFree (fm);

    ext_list = MakeExtensionList (i18n_core,
                                  query_ext->extension,
                                  number,
                                  &reply_number);

    for (i = 0;  i < number;  i++)
        XFree (query_ext->extension[i].name);
    /*endfor*/
    XFree (query_ext->extension);

    fm = FrameMgrInit (query_extension_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* set iteration count for list of extensions */
    FrameMgrSetIterCount (fm, reply_number);

    /* set length of BARRAY item in ext_fr */
    for (i = 0;  i < reply_number;  i++)
    {
        str_size = strlen (ext_list[i].name);
        FrameMgrSetSize (fm, str_size);
    }
    /*endfor*/

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims,
                           connect_id,
                           XIM_ERROR,
                           0,
                           0,
                           0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);

    for (i = 0;  i < reply_number;  i++)
    {
        str_size = FrameMgrGetSize (fm);
        FrameMgrPutToken (fm, ext_list[i].major_opcode);
        FrameMgrPutToken (fm, ext_list[i].minor_opcode);
        FrameMgrPutToken (fm, str_size);
        FrameMgrPutToken (fm, ext_list[i].name);
    }
    /*endfor*/
    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_QUERY_EXTENSION_REPLY,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    for (i = 0;  i < reply_number;  i++)
        XFree (ext_list[i].name);
    /*endfor*/
    XFree ((char *) ext_list);
}

static void SyncReplyMessageProc (XIMS ims,
                                  IMProtocol *call_data,
                                  unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec sync_reply_fr[];
    CARD16 connect_id = call_data->any.connect_id;
    Xi18nClient *client;
    CARD16 input_method_ID;
    CARD16 input_context_ID;

    client = (Xi18nClient *)_Xi18nFindClient (i18n_core, connect_id);
    fm = FrameMgrInit (sync_reply_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, input_context_ID);
    FrameMgrFree (fm);

    client->sync = False;

    if (ims->sync == True) {
	ims->sync = False;
	if (i18n_core->address.improto) {
	    call_data->sync_xlib.major_code = XIM_SYNC_REPLY;
	    call_data->sync_xlib.minor_code = 0;
	    call_data->sync_xlib.connect_id = input_method_ID;
	    call_data->sync_xlib.icid = input_context_ID;
	    i18n_core->address.improto(ims, call_data);
	}
    }
}

static void GetIMValueFromName (Xi18n i18n_core,
                                CARD16 connect_id,
                                char *buf,
                                char *name,
                                int *length)
{
    register int i;

    if (strcmp (name, XNQueryInputStyle) == 0)
    {
        XIMStyles *styles = (XIMStyles *) &i18n_core->address.input_styles;

        *length = sizeof (CARD16)*2; 	/* count_styles, unused */
        *length += styles->count_styles*sizeof (CARD32);

        if (buf != NULL)
        {
            FrameMgr fm;
            extern XimFrameRec input_styles_fr[];
            unsigned char *data = NULL;
            int total_size;
            
            fm = FrameMgrInit (input_styles_fr,
                               NULL,
                               _Xi18nNeedSwap (i18n_core, connect_id));

            /* set iteration count for list of input_style */
            FrameMgrSetIterCount (fm, styles->count_styles);

            total_size = FrameMgrGetTotalSize (fm);
            data = (unsigned char *) malloc (total_size);
            if (!data)
                return;
            /*endif*/
            memset (data, 0, total_size);
            FrameMgrSetBuffer (fm, data);

            FrameMgrPutToken (fm, styles->count_styles);
            for (i = 0;  i < (int) styles->count_styles;  i++)
                FrameMgrPutToken (fm, styles->supported_styles[i]);
            /*endfor*/
            memmove (buf, data, total_size);
            FrameMgrFree (fm);
        }
        /*endif*/
    }
    /*endif*/

    else if (strcmp (name, XNQueryIMValuesList) == 0) {
    }
}

static XIMAttribute *MakeIMAttributeList (Xi18n i18n_core,
                                          CARD16 connect_id,
                                          CARD16 *list,
                                          int *number,
                                          int *length)
{
    XIMAttribute *attrib_list;
    int list_num;
    XIMAttr *attr = i18n_core->address.xim_attr;
    int list_len = i18n_core->address.im_attr_num;
    register int i;
    register int j;
    int value_length;
    int number_ret = 0;

    *length = 0;
    list_num = 0;
    for (i = 0;  i < *number;  i++)
    {
        for (j = 0;  j < list_len;  j++)
        {
            if (attr[j].attribute_id == list[i])
            {
                list_num++;
                break;
            }
            /*endif*/
        }
        /*endfor*/
    }
    /*endfor*/
    attrib_list = (XIMAttribute *) malloc (sizeof (XIMAttribute)*list_num);
    if (!attrib_list)
        return NULL;
    /*endif*/
    memset (attrib_list, 0, sizeof (XIMAttribute)*list_num);
    number_ret = list_num;
    list_num = 0;
    for (i = 0;  i < *number;  i++)
    {
        for (j = 0;  j < list_len;  j++)
        {
            if (attr[j].attribute_id == list[i])
            {
                attrib_list[list_num].attribute_id = attr[j].attribute_id;
                attrib_list[list_num].name_length = attr[j].length;
                attrib_list[list_num].name = attr[j].name;
                attrib_list[list_num].type = attr[j].type;
                GetIMValueFromName (i18n_core,
                                    connect_id,
                                    NULL,
                                    attr[j].name,
                                    &value_length);
                attrib_list[list_num].value_length = value_length;
                attrib_list[list_num].value = (void *) malloc (value_length);
                memset(attrib_list[list_num].value, 0, value_length);
                GetIMValueFromName (i18n_core,
                                    connect_id,
                                    attrib_list[list_num].value,
                                    attr[j].name,
                                    &value_length);
                *length += sizeof (CARD16)*2;
                *length += value_length;
                *length += IMPAD (value_length);
                list_num++;
                break;
            }
            /*endif*/
        }
        /*endfor*/
    }
    /*endfor*/
    *number = number_ret;
    return attrib_list;
}

static void GetIMValuesMessageProc (XIMS ims,
                                    IMProtocol *call_data,
                                    unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    FmStatus status;
    extern XimFrameRec get_im_values_fr[];
    extern XimFrameRec get_im_values_reply_fr[];
    CARD16 byte_length;
    int list_len, total_size;
    unsigned char *reply = NULL;
    int iter_count;
    register int i;
    register int j;
    int number;
    CARD16 *im_attrID_list;
    char **name_list;
    CARD16 name_number;
    XIMAttribute *im_attribute_list;
    IMGetIMValuesStruct *getim = (IMGetIMValuesStruct *)&call_data->getim;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    /* create FrameMgr */
    fm = FrameMgrInit (get_im_values_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, byte_length);
    im_attrID_list = (CARD16 *) malloc (sizeof (CARD16)*20);
    memset (im_attrID_list, 0, sizeof (CARD16)*20);
    name_list = (char **)malloc(sizeof(char *) * 20);
    memset(name_list, 0, sizeof(char *) * 20);
    number = 0;
    while (FrameMgrIsIterLoopEnd (fm, &status) == False)
    {
        FrameMgrGetToken (fm, im_attrID_list[number]);
        number++;
    }
    FrameMgrFree (fm);

    name_number = 0;
    for (i = 0;  i < number;  i++) {
        for (j = 0;  j < i18n_core->address.im_attr_num;  j++) {
            if (i18n_core->address.xim_attr[j].attribute_id ==
                    im_attrID_list[i]) {
                name_list[name_number++] = 
			i18n_core->address.xim_attr[j].name;
                break;
            }
        }
    }
    getim->number = name_number;
    getim->im_attr_list = name_list;
    XFree (name_list);


#ifdef PROTOCOL_RICH
    if (i18n_core->address.improto) {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
    }
#endif  /* PROTOCOL_RICH */

    im_attribute_list = MakeIMAttributeList (i18n_core,
                                             connect_id,
                                             im_attrID_list,
                                             &number,
                                             &list_len);
    if (im_attrID_list)
        XFree (im_attrID_list);
    /*endif*/

    fm = FrameMgrInit (get_im_values_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    iter_count = number;

    /* set iteration count for list of im_attribute */
    FrameMgrSetIterCount (fm, iter_count);

    /* set length of BARRAY item in ximattribute_fr */
    for (i = 0;  i < iter_count;  i++)
        FrameMgrSetSize (fm, im_attribute_list[i].value_length);
    /*endfor*/
    
    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);

    for (i = 0;  i < iter_count;  i++)
    {
        FrameMgrPutToken (fm, im_attribute_list[i].attribute_id);
        FrameMgrPutToken (fm, im_attribute_list[i].value_length);
        FrameMgrPutToken (fm, im_attribute_list[i].value);
    }
    /*endfor*/
    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_GET_IM_VALUES_REPLY,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    for (i = 0; i < iter_count; i++)
        XFree(im_attribute_list[i].value);
    XFree (im_attribute_list);
}

static void CreateICMessageProc (XIMS ims,
                                 IMProtocol *call_data,
                                 unsigned char *p)
{
    _Xi18nChangeIC (ims, call_data, p, True);
}

static void SetICValuesMessageProc (XIMS ims,
                                    IMProtocol *call_data,
                                    unsigned char *p)
{
    _Xi18nChangeIC (ims, call_data, p, False);
}

static void GetICValuesMessageProc (XIMS ims,
                                    IMProtocol *call_data,
                                    unsigned char *p)
{
    _Xi18nGetIC (ims, call_data, p);
}

static void SetICFocusMessageProc (XIMS ims,
                                   IMProtocol *call_data,
                                   unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec set_ic_focus_fr[];
    IMChangeFocusStruct *setfocus;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    setfocus = (IMChangeFocusStruct *) &call_data->changefocus;

    fm = FrameMgrInit (set_ic_focus_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, setfocus->icid);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
}

static void UnsetICFocusMessageProc (XIMS ims,
                                     IMProtocol *call_data,
                                     unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec unset_ic_focus_fr[];
    IMChangeFocusStruct *unsetfocus;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    unsetfocus = (IMChangeFocusStruct *) &call_data->changefocus;

    fm = FrameMgrInit (unset_ic_focus_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, unsetfocus->icid);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
}

static void DestroyICMessageProc (XIMS ims,
                                  IMProtocol *call_data,
                                  unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec destroy_ic_fr[];
    extern XimFrameRec destroy_ic_reply_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMDestroyICStruct *destroy =
        (IMDestroyICStruct *) &call_data->destroyic;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (destroy_ic_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, destroy->icid);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
    
    fm = FrameMgrInit (destroy_ic_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, destroy->icid);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_DESTROY_IC_REPLY,
                       0,
                       reply,
                       total_size);
    XFree(reply);
    FrameMgrFree (fm);
}

static void ResetICMessageProc (XIMS ims,
                                IMProtocol *call_data,
                                unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec reset_ic_fr[];
    extern XimFrameRec reset_ic_reply_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMResetICStruct *resetic =
        (IMResetICStruct *) &call_data->resetic;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (reset_ic_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, resetic->icid);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
    
    /* create FrameMgr */
    fm = FrameMgrInit (reset_ic_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* set length of STRING8 */
    FrameMgrSetSize (fm, resetic->length);

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, resetic->icid);
    FrameMgrPutToken(fm, resetic->length);
    FrameMgrPutToken (fm, resetic->commit_string);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_RESET_IC_REPLY,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree(reply);
}

static int WireEventToEvent (Xi18n i18n_core,
                             xEvent *event,
                             CARD16 serial,
                             XEvent *ev)
{
    ev->xany.serial = event->u.u.sequenceNumber & ((unsigned long) 0xFFFF);
    ev->xany.serial |= serial << 16;
    ev->xany.send_event = False;
    ev->xany.display = i18n_core->address.dpy;
    switch (ev->type = event->u.u.type & 0x7F)
    {
    case KeyPress:
    case KeyRelease:
        ((XKeyEvent *) ev)->keycode = event->u.u.detail;
        ((XKeyEvent *) ev)->window = event->u.keyButtonPointer.event;
        ((XKeyEvent *) ev)->state = event->u.keyButtonPointer.state;
        ((XKeyEvent *) ev)->time = event->u.keyButtonPointer.time;
        ((XKeyEvent *) ev)->root = event->u.keyButtonPointer.root;
        ((XKeyEvent *) ev)->x = event->u.keyButtonPointer.eventX;
        ((XKeyEvent *) ev)->y = event->u.keyButtonPointer.eventY;
        ((XKeyEvent *) ev)->x_root = 0;
        ((XKeyEvent *) ev)->y_root = 0;
        return True;
    }
    return False;
}

static void ForwardEventMessageProc (XIMS ims,
                                     IMProtocol *call_data,
                                     unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec forward_event_fr[];
    xEvent wire_event;
    IMForwardEventStruct *forward =
        (IMForwardEventStruct*) &call_data->forwardevent;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (forward_event_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, forward->icid);
    FrameMgrGetToken (fm, forward->sync_bit);
    FrameMgrGetToken (fm, forward->serial_number);
    p += sizeof (CARD16)*4;
    memmove (&wire_event, p, sizeof (xEvent));

    FrameMgrFree (fm);

    if (WireEventToEvent (i18n_core,
                          &wire_event,
                          forward->serial_number,
                          &forward->event) == True)
    {
        if (i18n_core->address.improto)
        {
            if (!(i18n_core->address.improto(ims, call_data)))
                return;
            /*endif*/
        }
        /*endif*/
    }
    /*endif*/
}

static void ExtForwardKeyEventMessageProc (XIMS ims,
                                           IMProtocol *call_data,
                                           unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec ext_forward_keyevent_fr[];
    CARD8 type, keycode;
    CARD16 state;
    CARD32 ev_time, window;
    IMForwardEventStruct *forward =
        (IMForwardEventStruct *) &call_data->forwardevent;
    XEvent *ev = (XEvent *) &forward->event;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (ext_forward_keyevent_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, forward->icid);
    FrameMgrGetToken (fm, forward->sync_bit);
    FrameMgrGetToken (fm, forward->serial_number);
    FrameMgrGetToken (fm, type);
    FrameMgrGetToken (fm, keycode);
    FrameMgrGetToken (fm, state);
    FrameMgrGetToken (fm, ev_time);
    FrameMgrGetToken (fm, window);

    FrameMgrFree (fm);

    if (type != KeyPress)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    
    /* make a faked keypress event */
    ev->type = (int)type;
    ev->xany.send_event = True;
    ev->xany.display = i18n_core->address.dpy;
    ev->xany.serial = (unsigned long) forward->serial_number;
    ((XKeyEvent *) ev)->keycode = (unsigned int) keycode;
    ((XKeyEvent *) ev)->state = (unsigned int) state;
    ((XKeyEvent *) ev)->time = (Time) ev_time;
    ((XKeyEvent *) ev)->window = (Window) window;
    ((XKeyEvent *) ev)->root = DefaultRootWindow (ev->xany.display);
    ((XKeyEvent *) ev)->x = 0;
    ((XKeyEvent *) ev)->y = 0;
    ((XKeyEvent *) ev)->x_root = 0;
    ((XKeyEvent *) ev)->y_root = 0;

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
}

static void ExtMoveMessageProc (XIMS ims,
                                IMProtocol *call_data,
                                unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec ext_move_fr[];
    IMMoveStruct *extmove =
        (IMMoveStruct*) & call_data->extmove;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (ext_move_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, extmove->icid);
    FrameMgrGetToken (fm, extmove->x);
    FrameMgrGetToken (fm, extmove->y);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
}

static void ExtensionMessageProc (XIMS ims,
                                  IMProtocol *call_data,
                                  unsigned char *p)
{
    switch (call_data->any.minor_code)
    {
    case XIM_EXT_FORWARD_KEYEVENT:
        ExtForwardKeyEventMessageProc (ims, call_data, p);
        break;

    case XIM_EXT_MOVE:
        ExtMoveMessageProc (ims, call_data, p);
        break;
    }
    /*endswitch*/
}

static void TriggerNotifyMessageProc (XIMS ims,
                                      IMProtocol *call_data,
                                      unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec trigger_notify_fr[], trigger_notify_reply_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMTriggerNotifyStruct *trigger =
        (IMTriggerNotifyStruct *) &call_data->triggernotify;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;
    CARD32 flag;

    fm = FrameMgrInit (trigger_notify_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, trigger->icid);
    FrameMgrGetToken (fm, trigger->flag);
    FrameMgrGetToken (fm, trigger->key_index);
    FrameMgrGetToken (fm, trigger->event_mask);
    /*
      In order to support Front End Method, this event_mask must be saved
      per clients so that it should be restored by an XIM_EXT_SET_EVENT_MASK
      call when preediting mode is reset to off.
     */

    flag = trigger->flag;

    FrameMgrFree (fm);

    fm = FrameMgrInit (trigger_notify_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));
 
    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, trigger->icid);

    /* NOTE:
       XIM_TRIGGER_NOTIFY_REPLY should be sent before XIM_SET_EVENT_MASK
       in case of XIM_TRIGGER_NOTIFY(flag == ON), while it should be
       sent after XIM_SET_EVENT_MASK in case of
       XIM_TRIGGER_NOTIFY(flag == OFF).
       */
    if (flag == 0)
    {
        /* on key */
        _Xi18nSendMessage (ims,
                           connect_id,
                           XIM_TRIGGER_NOTIFY_REPLY,
                           0,
                           reply,
                           total_size);
        IMPreeditStart (ims, (XPointer)call_data);
    }
    /*endif*/
    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/

    if (flag == 1)
    {
        /* off key */
        IMPreeditEnd (ims, (XPointer) call_data);
        _Xi18nSendMessage (ims,
                           connect_id,
                           XIM_TRIGGER_NOTIFY_REPLY,
                           0,
                           reply,
                           total_size);
    }
    /*endif*/
    FrameMgrFree (fm);
    XFree (reply);
}

static INT16 ChooseEncoding (Xi18n i18n_core,
                             IMEncodingNegotiationStruct *enc_nego)
{
    Xi18nAddressRec *address = (Xi18nAddressRec *) & i18n_core->address;
    XIMEncodings *p;
    int i, j;
    int enc_index=0;

    p = (XIMEncodings *) &address->encoding_list;
    for (i = 0;  i < (int) p->count_encodings;  i++)
    {
        for (j = 0;  j < (int) enc_nego->encoding_number;  j++)
        {
            if (strcmp (p->supported_encodings[i],
                        enc_nego->encoding[j].name) == 0)
            {
                enc_index = j;
                break;
            }
            /*endif*/
        }
        /*endfor*/
    }
    /*endfor*/

    return (INT16) enc_index;
#if 0
    return (INT16) XIM_Default_Encoding_IDX;
#endif
}

static void EncodingNegotiatonMessageProc (XIMS ims,
                                           IMProtocol *call_data,
                                           unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    FmStatus status;
    CARD16 byte_length;
    extern XimFrameRec encoding_negotiation_fr[];
    extern XimFrameRec encoding_negotiation_reply_fr[];
    register int i, total_size;
    unsigned char *reply = NULL;
    IMEncodingNegotiationStruct *enc_nego =
        (IMEncodingNegotiationStruct *) &call_data->encodingnego;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (encoding_negotiation_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    FrameMgrGetToken (fm, input_method_ID);

    /* get ENCODING STR field */
    FrameMgrGetToken (fm, byte_length);
    if (byte_length > 0)
    {
        enc_nego->encoding = (XIMStr *) malloc (sizeof (XIMStr)*10);
        memset (enc_nego->encoding, 0, sizeof (XIMStr)*10);
        i = 0;
        while (FrameMgrIsIterLoopEnd (fm, &status) == False)
        {
            char *name;
            int str_length;
            
            FrameMgrGetToken (fm, str_length);
            FrameMgrSetSize (fm, str_length);
            enc_nego->encoding[i].length = str_length;
            FrameMgrGetToken (fm, name);
            enc_nego->encoding[i].name = malloc (str_length + 1);
            strncpy (enc_nego->encoding[i].name, name, str_length);
            enc_nego->encoding[i].name[str_length] = '\0';
            i++;
        }
        /*endwhile*/
        enc_nego->encoding_number = i;
    }
    /*endif*/
    /* get ENCODING INFO field */
    FrameMgrGetToken (fm, byte_length);
    if (byte_length > 0)
    {
        enc_nego->encodinginfo = (XIMStr *) malloc (sizeof (XIMStr)*10);
        memset (enc_nego->encoding, 0, sizeof (XIMStr)*10);
        i = 0;
        while (FrameMgrIsIterLoopEnd (fm, &status) == False)
        {
            char *name;
            int str_length;
            
            FrameMgrGetToken (fm, str_length);
            FrameMgrSetSize (fm, str_length);
            enc_nego->encodinginfo[i].length = str_length;
            FrameMgrGetToken (fm, name);
            enc_nego->encodinginfo[i].name = malloc (str_length + 1);
            strncpy (enc_nego->encodinginfo[i].name, name, str_length);
            enc_nego->encodinginfo[i].name[str_length] = '\0';
            i++;
        }
        /*endwhile*/
        enc_nego->encoding_info_number = i;
    }
    /*endif*/

    enc_nego->enc_index = ChooseEncoding (i18n_core, enc_nego);
    enc_nego->category = 0;

#ifdef PROTOCOL_RICH
    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
#endif  /* PROTOCOL_RICH */

    FrameMgrFree (fm);

    fm = FrameMgrInit (encoding_negotiation_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, enc_nego->category);
    FrameMgrPutToken (fm, enc_nego->enc_index);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_ENCODING_NEGOTIATION_REPLY,
                       0,
                       reply,
                       total_size);
    XFree (reply);

    /* free data for encoding list */
    if (enc_nego->encoding)
    {
        for (i = 0;  i < (int) enc_nego->encoding_number;  i++)
            XFree (enc_nego->encoding[i].name);
        /*endfor*/
        XFree (enc_nego->encoding);
    }
    /*endif*/
    if (enc_nego->encodinginfo)
    {
        for (i = 0;  i < (int) enc_nego->encoding_info_number;  i++)
            XFree (enc_nego->encodinginfo[i].name);
        /*endfor*/
        XFree (enc_nego->encodinginfo);
    }
    /*endif*/
    FrameMgrFree (fm);
}

void PreeditStartReplyMessageProc (XIMS ims,
                                   IMProtocol *call_data,
                                   unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec preedit_start_reply_fr[];
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct *) &call_data->preedit_callback;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (preedit_start_reply_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, preedit_CB->icid);
    FrameMgrGetToken (fm, preedit_CB->todo.return_value);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
}

void PreeditCaretReplyMessageProc (XIMS ims,
                                   IMProtocol *call_data,
                                   unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec preedit_caret_reply_fr[];
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct *) &call_data->preedit_callback;
    XIMPreeditCaretCallbackStruct *caret =
        (XIMPreeditCaretCallbackStruct *) & preedit_CB->todo.caret;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    fm = FrameMgrInit (preedit_caret_reply_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, preedit_CB->icid);
    FrameMgrGetToken (fm, caret->position);

    FrameMgrFree (fm);

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
}

void StrConvReplyMessageProc (XIMS ims,
                              IMProtocol *call_data,
                              unsigned char *p)
{
    return;
}

static void AddQueue (Xi18nClient *client, unsigned char *p)
{
    XIMPending *new;
    XIMPending *last;

    if ((new = (XIMPending *) malloc (sizeof (XIMPending))) == NULL)
        return;
    /*endif*/
    new->p = p;
    new->next = (XIMPending *) NULL;
    if (!client->pending)
    {
        client->pending = new;
    }
    else
    {
        for (last = client->pending;  last->next;  last = last->next)
            ;
        /*endfor*/
        last->next = new;
    }
    /*endif*/
    return;
}

static void ProcessQueue (XIMS ims, CARD16 connect_id)
{
    Xi18n i18n_core = ims->protocol;
    Xi18nClient *client = (Xi18nClient *) _Xi18nFindClient (i18n_core,
                                                            connect_id);

    while (client->sync == False  &&  client->pending)
    {
        XimProtoHdr *hdr = (XimProtoHdr *) client->pending->p;
        unsigned char *p1 = (unsigned char *) (hdr + 1);
        IMProtocol call_data;

        call_data.major_code = hdr->major_opcode;
        call_data.any.minor_code = hdr->minor_opcode;
        call_data.any.connect_id = connect_id;

        switch (hdr->major_opcode)
        {
        case XIM_FORWARD_EVENT:
            ForwardEventMessageProc(ims, &call_data, p1);
            break;
        }
        /*endswitch*/
        XFree (hdr);
        {
            XIMPending *old = client->pending;

            client->pending = old->next;
            XFree (old);
        }
    }
    /*endwhile*/
    return;
}


void _Xi18nMessageHandler (XIMS ims,
                           CARD16 connect_id,
                           unsigned char *p,
                           Bool *delete)
{
    XimProtoHdr	*hdr = (XimProtoHdr *)p;
    unsigned char *p1 = (unsigned char *)(hdr + 1);
    IMProtocol call_data;
    Xi18n i18n_core = ims->protocol;
    Xi18nClient *client;

    client = (Xi18nClient *) _Xi18nFindClient (i18n_core, connect_id);
    if (hdr == (XimProtoHdr *) NULL)
        return;
    /*endif*/
    
    memset (&call_data, 0, sizeof(IMProtocol));

    call_data.major_code = hdr->major_opcode;
    call_data.any.minor_code = hdr->minor_opcode;
    call_data.any.connect_id = connect_id;

    switch (call_data.major_code)
    {
    case XIM_CONNECT:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_CONNECT\n");
#endif
        ConnectMessageProc (ims, &call_data, p1);
        break;

    case XIM_DISCONNECT:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_DISCONNECT\n");
#endif
        DisConnectMessageProc (ims, &call_data);
        break;

    case XIM_OPEN:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_OPEN\n");
#endif
        OpenMessageProc (ims, &call_data, p1);
        break;

    case XIM_CLOSE:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_CLOSE\n");
#endif
        CloseMessageProc (ims, &call_data, p1);
        break;

    case XIM_QUERY_EXTENSION:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_QUERY_EXTENSION\n");
#endif
        QueryExtensionMessageProc (ims, &call_data, p1);
        break;

    case XIM_GET_IM_VALUES:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_GET_IM_VALUES\n");
#endif
        GetIMValuesMessageProc (ims, &call_data, p1);
        break;

    case XIM_CREATE_IC:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_CREATE_IC\n");
#endif
        CreateICMessageProc (ims, &call_data, p1);
        break;

    case XIM_SET_IC_VALUES:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_SET_IC_VALUES\n");
#endif
        SetICValuesMessageProc (ims, &call_data, p1);
        break;

    case XIM_GET_IC_VALUES:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_GET_IC_VALUES\n");
#endif
        GetICValuesMessageProc (ims, &call_data, p1);
        break;

    case XIM_SET_IC_FOCUS:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_SET_IC_FOCUS\n");
#endif
        SetICFocusMessageProc (ims, &call_data, p1);
        break;

    case XIM_UNSET_IC_FOCUS:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_UNSET_IC_FOCUS\n");
#endif
        UnsetICFocusMessageProc (ims, &call_data, p1);
        break;

    case XIM_DESTROY_IC:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_DESTROY_IC\n");
#endif
        DestroyICMessageProc (ims, &call_data, p1);
        break;

    case XIM_RESET_IC:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_RESET_IC\n");
#endif
        ResetICMessageProc (ims, &call_data, p1);
        break;

    case XIM_FORWARD_EVENT:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_FORWARD_EVENT\n");
#endif
        if (client->sync == True)
        {
            AddQueue (client, p);
            *delete = False;
        }
        else
        {
            ForwardEventMessageProc (ims, &call_data, p1);
        }
        break;

    case XIM_EXTENSION:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_EXTENSION\n");
#endif
        ExtensionMessageProc (ims, &call_data, p1);
        break;

    case XIM_SYNC:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_SYNC\n");
#endif
        break;

    case XIM_SYNC_REPLY:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_SYNC_REPLY\n");
#endif
        SyncReplyMessageProc (ims, &call_data, p1);
        ProcessQueue (ims, connect_id);
        break;

    case XIM_TRIGGER_NOTIFY:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_TRIGGER_NOTIFY\n");
#endif
        TriggerNotifyMessageProc (ims, &call_data, p1);
        break;

    case XIM_ENCODING_NEGOTIATION:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_ENCODING_NEGOTIATION\n");
#endif
        EncodingNegotiatonMessageProc (ims, &call_data, p1);
        break;

    case XIM_PREEDIT_START_REPLY:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_PREEDIT_START_REPLY\n");
#endif
        PreeditStartReplyMessageProc (ims, &call_data, p1);
        break;

    case XIM_PREEDIT_CARET_REPLY:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_PREEDIT_CARET_REPLY\n");
#endif
        PreeditCaretReplyMessageProc (ims, &call_data, p1);
        break;

    case XIM_STR_CONVERSION_REPLY:
#ifdef XIM_DEBUG
	DebugLog("-- XIM_STR_CONVERSION_REPLY\n");
#endif
        StrConvReplyMessageProc (ims, &call_data, p1);
        break;
    }
    /*endswitch*/
}
