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

#include <X11/Xlib.h>
#include "IMdkit.h"
#include "Xi18n.h"
#include "FrameMgr.h"
#include "XimFunc.h"

Xi18nClient *_Xi18nFindClient (Xi18n, CARD16);

int
_Xi18nNeedSwap (Xi18n i18n_core, CARD16 connect_id)
{
    CARD8 im_byteOrder = i18n_core->address.im_byteOrder;
    Xi18nClient *client = _Xi18nFindClient (i18n_core, connect_id);

    return (client->byte_order != im_byteOrder);
}

Xi18nClient *_Xi18nNewClient(Xi18n i18n_core)
{
    static CARD16 connect_id = 0;
    int new_connect_id;
    Xi18nClient *client;

    if (i18n_core->address.free_clients)
    {
        client = i18n_core->address.free_clients;
        i18n_core->address.free_clients = client->next;
	new_connect_id = client->connect_id;
    }
    else
    {
        client = (Xi18nClient *) malloc (sizeof (Xi18nClient));
	new_connect_id = ++connect_id;
    }
    /*endif*/
    memset (client, 0, sizeof (Xi18nClient));
    client->connect_id = new_connect_id;
    client->pending = (XIMPending *) NULL;
    client->sync = False;
    client->byte_order = '?'; 	/* initial value */
    memset (&client->pending, 0, sizeof (XIMPending *));
    client->next = i18n_core->address.clients;
    i18n_core->address.clients = client;

    return (Xi18nClient *) client;
}

Xi18nClient *_Xi18nFindClient (Xi18n i18n_core, CARD16 connect_id)
{
    Xi18nClient *client = i18n_core->address.clients;

    while (client)
    {
        if (client->connect_id == connect_id)
            return client;
        /*endif*/
        client = client->next;
    }
    /*endwhile*/
    return NULL;
}

void _Xi18nDeleteClient (Xi18n i18n_core, CARD16 connect_id)
{
    Xi18nClient *target = _Xi18nFindClient (i18n_core, connect_id);
    Xi18nClient *ccp;
    Xi18nClient *ccp0;

    for (ccp = i18n_core->address.clients, ccp0 = NULL;
         ccp != NULL;
         ccp0 = ccp, ccp = ccp->next)
    {
        if (ccp == target)
        {
            if (ccp0 == NULL)
                i18n_core->address.clients = ccp->next;
            else
                ccp0->next = ccp->next;
            /*endif*/
            /* put it back to free list */
            target->next = i18n_core->address.free_clients;
            i18n_core->address.free_clients = target;
            return;
        }
        /*endif*/
    }
    /*endfor*/
}

void _Xi18nSendMessage (XIMS ims,
                        CARD16 connect_id,
                        CARD8 major_opcode,
                        CARD8 minor_opcode,
                        unsigned char *data,
                        long length)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec packet_header_fr[];
    unsigned char *reply_hdr = NULL;
    int header_size;
    unsigned char *reply = NULL;
    unsigned char *replyp;
    int reply_length;
    long p_len = length/4;

    fm = FrameMgrInit (packet_header_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    header_size = FrameMgrGetTotalSize (fm);
    reply_hdr = (unsigned char *) malloc (header_size);
    if (reply_hdr == NULL)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    FrameMgrSetBuffer (fm, reply_hdr);

    /* put data */
    FrameMgrPutToken (fm, major_opcode);
    FrameMgrPutToken (fm, minor_opcode);
    FrameMgrPutToken (fm, p_len);

    reply_length = header_size + length;
    reply = (unsigned char *) malloc (reply_length);
    replyp = reply;
    memmove (reply, reply_hdr, header_size);
    replyp += header_size;
    memmove (replyp, data, length);

    i18n_core->methods.send (ims, connect_id, reply, reply_length);

    XFree (reply);
    XFree (reply_hdr);
    FrameMgrFree (fm);
}

void _Xi18nSendTriggerKey (XIMS ims, CARD16 connect_id)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec register_triggerkeys_fr[];
    XIMTriggerKey *on_keys = i18n_core->address.on_keys.keylist;
    XIMTriggerKey *off_keys = i18n_core->address.off_keys.keylist;
    int on_key_num = i18n_core->address.on_keys.count_keys;
    int off_key_num = i18n_core->address.off_keys.count_keys;
    unsigned char *reply = NULL;
    register int i, total_size;
    CARD16 im_id;

    if (on_key_num == 0  &&  off_key_num == 0)
        return;
    /*endif*/
    
    fm = FrameMgrInit (register_triggerkeys_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* set iteration count for on-keys list */
    FrameMgrSetIterCount (fm, on_key_num);
    /* set iteration count for off-keys list */
    FrameMgrSetIterCount (fm, off_key_num);

    /* get total_size */
    total_size = FrameMgrGetTotalSize (fm);

    reply = (unsigned char *) malloc (total_size);
    if (!reply)
        return;
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    /* Right now XIM_OPEN_REPLY hasn't been sent to this new client, so
       the input-method-id is still invalid, and should be set to zero...
       Reter to $(XC)/lib/X11/imDefLkup.c:_XimRegisterTriggerKeysCallback
     */
    im_id = 0;
    FrameMgrPutToken (fm, im_id);  /* input-method-id */
    for (i = 0;  i < on_key_num;  i++)
    {
        FrameMgrPutToken (fm, on_keys[i].keysym);
        FrameMgrPutToken (fm, on_keys[i].modifier);
        FrameMgrPutToken (fm, on_keys[i].modifier_mask);
    }
    /*endfor*/
    for (i = 0;  i < off_key_num;  i++)
    {
        FrameMgrPutToken (fm, off_keys[i].keysym);
        FrameMgrPutToken (fm, off_keys[i].modifier);
        FrameMgrPutToken (fm, off_keys[i].modifier_mask);
    }
    /*endfor*/
    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_REGISTER_TRIGGERKEYS,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree(reply);
}

void _Xi18nSetEventMask (XIMS ims,
                         CARD16 connect_id,
                         CARD16 im_id,
                         CARD16 ic_id,
                         CARD32 forward_mask,
                         CARD32 sync_mask)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec set_event_mask_fr[];
    unsigned char *reply = NULL;
    register int total_size;

    fm = FrameMgrInit (set_event_mask_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
        return;
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, im_id); 	/* input-method-id */
    FrameMgrPutToken (fm, ic_id); 	/* input-context-id */
    FrameMgrPutToken (fm, forward_mask);
    FrameMgrPutToken (fm, sync_mask);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_SET_EVENT_MASK,
                       0,
                       reply,
                       total_size);

    FrameMgrFree (fm);
    XFree(reply);
}
