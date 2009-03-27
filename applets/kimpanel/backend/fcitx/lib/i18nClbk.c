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

int _Xi18nGeometryCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec geometry_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMGeometryCBStruct *geometry_CB =
        (IMGeometryCBStruct *) &call_data->geometry_callback;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (geometry_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, geometry_CB->icid);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_GEOMETRY,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    /* XIM_GEOMETRY is an asyncronous protocol,
       so return immediately. */
    return True;
}

int _Xi18nPreeditStartCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec preedit_start_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct*) &call_data->preedit_callback;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (preedit_start_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage(ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, preedit_CB->icid);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_PREEDIT_START,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    return True;
}

int _Xi18nPreeditDrawCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec preedit_draw_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct *) &call_data->preedit_callback;
    XIMPreeditDrawCallbackStruct *draw =
        (XIMPreeditDrawCallbackStruct *) &preedit_CB->todo.draw;
    CARD16 connect_id = call_data->any.connect_id;
    register int feedback_count;
    register int i;
    BITMASK32 status = 0x0;

    if (draw->text->length == 0)
        status = 0x00000001;
    else if (draw->text->feedback[0] == 0)
        status = 0x00000002;
    /*endif*/

    fm = FrameMgrInit (preedit_draw_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* set length of preedit string */
    FrameMgrSetSize (fm, draw->text->length);

    /* set iteration count for list of feedback */
    for (i = 0;  draw->text->feedback[i] != 0;  i++)
        ;
    /*endfor*/
    feedback_count = i;
    FrameMgrSetIterCount (fm, feedback_count);

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, preedit_CB->icid);
    FrameMgrPutToken (fm, draw->caret);
    FrameMgrPutToken (fm, draw->chg_first);
    FrameMgrPutToken (fm, draw->chg_length);
    FrameMgrPutToken (fm, status);
    FrameMgrPutToken (fm, draw->text->length);
    FrameMgrPutToken (fm, draw->text->string);
    for (i = 0;  i < feedback_count;  i++)
        FrameMgrPutToken (fm, draw->text->feedback[i]);
    /*endfor*/
    
    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_PREEDIT_DRAW,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    /* XIM_PREEDIT_DRAW is an asyncronous protocol, so return immediately. */
    return True;
}

int _Xi18nPreeditCaretCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec preedit_caret_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct*) &call_data->preedit_callback;
    XIMPreeditCaretCallbackStruct *caret =
        (XIMPreeditCaretCallbackStruct *) &preedit_CB->todo.caret;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (preedit_caret_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, preedit_CB->icid);
    FrameMgrPutToken (fm, caret->position);
    FrameMgrPutToken (fm, caret->direction);
    FrameMgrPutToken (fm, caret->style);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_PREEDIT_CARET,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    return True;
}

int _Xi18nPreeditDoneCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec preedit_done_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMPreeditCBStruct *preedit_CB =
        (IMPreeditCBStruct *) &call_data->preedit_callback;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (preedit_done_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, preedit_CB->icid);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_PREEDIT_DONE,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    /* XIM_PREEDIT_DONE is an asyncronous protocol, so return immediately. */
    return True;
}

int _Xi18nStatusStartCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec status_start_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMStatusCBStruct *status_CB =
        (IMStatusCBStruct*) &call_data->status_callback;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (status_start_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));
    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, status_CB->icid);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_STATUS_START,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    /* XIM_STATUS_START is an asyncronous protocol, so return immediately. */
    return True;
}

int _Xi18nStatusDrawCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm = (FrameMgr)0;
    extern XimFrameRec status_draw_text_fr[];
    extern XimFrameRec status_draw_bitmap_fr[];
    register int total_size = 0;
    unsigned char *reply = NULL;
    IMStatusCBStruct *status_CB =
        (IMStatusCBStruct *) &call_data->status_callback;
    XIMStatusDrawCallbackStruct *draw =
        (XIMStatusDrawCallbackStruct *) &status_CB->todo.draw;
    CARD16 connect_id = call_data->any.connect_id;
    register int feedback_count;
    register int i;
    BITMASK32 status = 0x0;

    switch (draw->type)
    {
    case XIMTextType:
        fm = FrameMgrInit (status_draw_text_fr,
                           NULL,
                           _Xi18nNeedSwap (i18n_core, connect_id));

        if (draw->data.text->length == 0)
            status = 0x00000001;
        else if (draw->data.text->feedback[0] == 0)
            status = 0x00000002;
        /*endif*/
        
        /* set length of status string */
        FrameMgrSetSize(fm, draw->data.text->length);
        /* set iteration count for list of feedback */
        for (i = 0;  draw->data.text->feedback[i] != 0;  i++)
            ;
        /*endfor*/
        feedback_count = i;
        FrameMgrSetIterCount (fm, feedback_count);

        total_size = FrameMgrGetTotalSize (fm);
        reply = (unsigned char *) malloc (total_size);
        if (!reply)
        {
            _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
            return False;
        }
        /*endif*/
        memset (reply, 0, total_size);
        FrameMgrSetBuffer (fm, reply);

        FrameMgrPutToken (fm, connect_id);
        FrameMgrPutToken (fm, status_CB->icid);
        FrameMgrPutToken (fm, draw->type);
        FrameMgrPutToken (fm, status);
        FrameMgrPutToken (fm, draw->data.text->length);
        FrameMgrPutToken (fm, draw->data.text->string);
        for (i = 0;  i < feedback_count;  i++)
            FrameMgrPutToken (fm, draw->data.text->feedback[i]);
        /*endfor*/
        break;

    case XIMBitmapType:
        fm = FrameMgrInit (status_draw_bitmap_fr,
                           NULL,
                           _Xi18nNeedSwap (i18n_core, connect_id));

        total_size = FrameMgrGetTotalSize (fm);
        reply = (unsigned char *) malloc (total_size);
        if (!reply)
        {
            _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
            return False;
        }
        /*endif*/
        memset (reply, 0, total_size);
        FrameMgrSetBuffer (fm, reply);

        FrameMgrPutToken (fm, connect_id);
        FrameMgrPutToken (fm, status_CB->icid);
        FrameMgrPutToken (fm, draw->data.bitmap);
        break;
    }
    /*endswitch*/
    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_STATUS_DRAW,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    /* XIM_STATUS_DRAW is an asyncronous protocol, so return immediately. */
    return True;
}

int _Xi18nStatusDoneCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec status_done_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMStatusCBStruct *status_CB =
        (IMStatusCBStruct *) &call_data->status_callback;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (status_done_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, status_CB->icid);

    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_STATUS_DONE,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    /* XIM_STATUS_DONE is an asyncronous protocol, so return immediately. */
    return True;
}

int _Xi18nStringConversionCallback (XIMS ims, IMProtocol *call_data)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    extern XimFrameRec str_conversion_fr[];
    register int total_size;
    unsigned char *reply = NULL;
    IMStrConvCBStruct *call_back =
        (IMStrConvCBStruct *) &call_data->strconv_callback;
    XIMStringConversionCallbackStruct *strconv =
        (XIMStringConversionCallbackStruct *) &call_back->strconv;
    CARD16 connect_id = call_data->any.connect_id;

    fm = FrameMgrInit (str_conversion_fr,
                       NULL,
                      _Xi18nNeedSwap (i18n_core, connect_id));
#if 0
    /* set length of preedit string */
    FrameMgrSetSize (fm, strconv->text->length);
#endif
    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (!reply)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return False;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, connect_id);
    FrameMgrPutToken (fm, call_back->icid);
    FrameMgrPutToken (fm, strconv->position);
    FrameMgrPutToken (fm, strconv->direction);
    FrameMgrPutToken (fm, strconv->operation);
#if 0
    FrameMgrPutToken (fm, strconv->text->string);
#endif
    _Xi18nSendMessage (ims, connect_id,
                       XIM_STR_CONVERSION,
                       0,
                       reply,
                       total_size);
    FrameMgrFree (fm);
    XFree (reply);

    /* XIM_STR_CONVERSION is a syncronous protocol,
       so should wait here for XIM_STR_CONVERSION_REPLY. */
    if (i18n_core->methods.wait (ims,
                                 connect_id,
                                 XIM_STR_CONVERSION_REPLY,
                                 0) == False)
    {
        return False;
    }
    /*endif*/
    return True;
}
