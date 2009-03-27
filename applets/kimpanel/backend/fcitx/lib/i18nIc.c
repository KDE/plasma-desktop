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

#define IC_SIZE 64

/* Set IC values */
static void SetCardAttribute (XICAttribute *value_ret,
                              char *p,
                              XICAttr *ic_attr,
                              int value_length,
                              int need_swap)
{
    char *buf;
    FrameMgr fm;

    if ((buf = (char *) malloc (value_length)) == NULL)
        return;
    /*endif*/
    if (value_length == sizeof (CARD8))
    {
        memmove (buf, p, value_length);
    }
    else if (value_length == sizeof (CARD16))
    {
        INT16 value;
        extern XimFrameRec short_fr[];

        fm = FrameMgrInit (short_fr, (char *) p, need_swap);
        /* get data */
        FrameMgrGetToken (fm, value);
        FrameMgrFree (fm);
        memmove (buf, &value, value_length);
    }
    else if (value_length == sizeof(CARD32))
    {
        INT32 value;
        extern XimFrameRec long_fr[];
        
        fm = FrameMgrInit (long_fr, (char *) p, need_swap);
        /* get data */
        FrameMgrGetToken (fm, value);
        FrameMgrFree (fm);
        memmove (buf, &value, value_length);
    }
    /*endif*/
    value_ret->attribute_id = ic_attr->attribute_id;
    value_ret->name = ic_attr->name;
    value_ret->name_length = ic_attr->length;
    value_ret->type = ic_attr->type;
    value_ret->value_length = value_length;
    value_ret->value = buf;
}

static void SetFontAttribute (XICAttribute *value_ret,
                              char *p,
                              XICAttr *ic_attr,
                              int value_length,
                              int need_swap)
{
    char *buf;
    char *base_name;
    CARD16 base_length;
    FrameMgr fm;
    extern XimFrameRec fontset_fr[];

    fm = FrameMgrInit (fontset_fr, (char *) p, need_swap);
    /* get data */
    FrameMgrGetToken (fm, base_length);
    FrameMgrSetSize (fm, base_length);
    if ((buf = (char *) malloc (base_length + 1)) == NULL)
        return;
    /*endif*/
    FrameMgrGetToken (fm, base_name);
    FrameMgrFree(fm);
    strncpy (buf, base_name, base_length);
    buf[base_length] = (char) 0;

    value_ret->attribute_id = ic_attr->attribute_id;
    value_ret->name = ic_attr->name;
    value_ret->name_length = ic_attr->length;
    value_ret->type = ic_attr->type;
    value_ret->value_length = value_length;
    value_ret->value = buf;
}

static void SetPointAttribute (XICAttribute *value_ret,
                               char *p,
                               XICAttr *ic_attr,
                               int value_length,
                               int need_swap)
{
    XPoint *buf;
    FrameMgr fm;
    extern XimFrameRec xpoint_fr[];

    if ((buf = (XPoint *) malloc (sizeof (XPoint))) == NULL)
        return;
    /*endif*/

    fm = FrameMgrInit (xpoint_fr, (char *) p, need_swap);
    /* get data */
    FrameMgrGetToken (fm, buf->x);
    FrameMgrGetToken (fm, buf->y);
    FrameMgrFree (fm);

    memmove (&(buf->x), p, sizeof (INT16));
    p += sizeof (INT16);
    memmove (&(buf->y), p, sizeof (INT16));

    value_ret->attribute_id = ic_attr->attribute_id;
    value_ret->name = ic_attr->name;
    value_ret->name_length = ic_attr->length;
    value_ret->type = ic_attr->type;
    value_ret->value_length = value_length;
    value_ret->value = (char *) buf;
}

static void SetRectAttribute (XICAttribute *value_ret,
                              char *p,
                              XICAttr *ic_attr,
                              int value_length,
                              int need_swap)
{
    XRectangle *buf;
    FrameMgr fm;
    extern XimFrameRec xrectangle_fr[];

    if ((buf = (XRectangle *) malloc (sizeof (XRectangle))) == NULL)
        return;
    /*endif*/
    
    fm = FrameMgrInit (xrectangle_fr, (char *) p, need_swap);
    /* get data */
    FrameMgrGetToken (fm, buf->x);
    FrameMgrGetToken (fm, buf->y);
    FrameMgrGetToken (fm, buf->width);
    FrameMgrGetToken (fm, buf->height);
    FrameMgrFree (fm);

    value_ret->attribute_id = ic_attr->attribute_id;
    value_ret->name = ic_attr->name;
    value_ret->name_length = ic_attr->length;
    value_ret->type = ic_attr->type;
    value_ret->value_length = value_length;
    value_ret->value = (char *) buf;

    return;
}

#if 0
static void SetHotKeyAttribute (XICAttribute *value_ret,
                                char *p,
                                XICAttr *ic_attr,
                                int value_length,
                                int need_swap)
{
    INT32 list_number;
    XIMTriggerKey *hotkeys;

    memmove (&list_number, p, sizeof(INT32)); p += sizeof(INT32);

    hotkeys = (XIMTriggerKey *) malloc (list_number*sizeof (XIMTriggerKey));
    if (hotkeys == NULL)
        return;
    /*endif*/
    
    memmove (hotkeys, p, list_number*sizeof (XIMTriggerKey));

    value_ret->attribute_id = ic_attr->attribute_id;
    value_ret->name = ic_attr->name;
    value_ret->name_length = ic_attr->length;
    value_ret->type = ic_attr->type;
    value_ret->value_length = value_length;
    value_ret->value = (char *) hotkeys;
}
#endif

/* get IC values */
static void GetAttrHeader (unsigned char *rec,
                           XICAttribute *list,
                           int need_swap)
{
    FrameMgr fm;
    extern XimFrameRec attr_head_fr[];

    fm = FrameMgrInit (attr_head_fr, (char *) rec, need_swap);
    /* put data */
    FrameMgrPutToken (fm, list->attribute_id);
    FrameMgrPutToken (fm, list->value_length);
    FrameMgrFree (fm);
}

static void GetCardAttribute (char *rec, XICAttribute *list, int need_swap)
{
    FrameMgr fm;
    unsigned char *recp = (unsigned char *) rec;

    GetAttrHeader (recp, list, need_swap);
    recp += sizeof (CARD16)*2;

    if (list->value_length == sizeof (CARD8))
    {
        memmove (recp, list->value, list->value_length);
    }
    else if (list->value_length == sizeof (CARD16))
    {
        INT16 *value = (INT16 *) list->value;
        extern XimFrameRec short_fr[];

        fm = FrameMgrInit (short_fr, (char *) recp, need_swap);
        /* put data */
        FrameMgrPutToken (fm, *value);
        FrameMgrFree (fm);
    }
    else if (list->value_length == sizeof (CARD32))
    {
        INT32 *value = (INT32 *) list->value;
        extern XimFrameRec long_fr[];

        fm = FrameMgrInit (long_fr, (char *) recp, need_swap);
        /* put data */
        FrameMgrPutToken (fm, *value);
        FrameMgrFree (fm);
    }
    /*endif*/
}

static void GetFontAttribute(char *rec, XICAttribute *list, int need_swap)
{
    FrameMgr fm;
    extern XimFrameRec fontset_fr[];
    char *base_name = (char *) list->value;
    unsigned char *recp = (unsigned char *) rec;

    GetAttrHeader (recp, list, need_swap);
    recp += sizeof (CARD16)*2;

    fm = FrameMgrInit (fontset_fr, (char *)recp, need_swap);
    /* put data */
    FrameMgrSetSize (fm, list->value_length);
    FrameMgrPutToken (fm, list->value_length);
    FrameMgrPutToken (fm, base_name);
    FrameMgrFree (fm);
}

static void GetRectAttribute (char *rec, XICAttribute *list, int need_swap)
{
    FrameMgr fm;
    extern XimFrameRec xrectangle_fr[];
    XRectangle *rect = (XRectangle *) list->value;
    unsigned char *recp = (unsigned char *) rec;

    GetAttrHeader (recp, list, need_swap);
    recp += sizeof(CARD16)*2;

    fm = FrameMgrInit (xrectangle_fr, (char *) recp, need_swap);
    /* put data */
    FrameMgrPutToken (fm, rect->x);
    FrameMgrPutToken (fm, rect->y);
    FrameMgrPutToken (fm, rect->width);
    FrameMgrPutToken (fm, rect->height);
    FrameMgrFree (fm);
}

static void GetPointAttribute (char *rec, XICAttribute *list, int need_swap)
{
    FrameMgr fm;
    extern XimFrameRec xpoint_fr[];
    XPoint *rect = (XPoint *) list->value;
    unsigned char *recp = (unsigned char *) rec;

    GetAttrHeader (recp, list, need_swap);
    recp += sizeof(CARD16)*2;

    fm = FrameMgrInit (xpoint_fr, (char *) recp, need_swap);
    /* put data */
    FrameMgrPutToken (fm, rect->x);
    FrameMgrPutToken (fm, rect->y);
    FrameMgrFree (fm);
}

static int ReadICValue (Xi18n i18n_core,
                        CARD16 icvalue_id,
                        int value_length,
                        void *p,
                        XICAttribute *value_ret,
                        CARD16 *number_ret,
                        int need_swap)
{
    XICAttr *ic_attr = i18n_core->address.xic_attr;
    int i;

    *number_ret = (CARD16) 0;

    for (i = 0;  i < i18n_core->address.ic_attr_num;  i++, ic_attr++)
    {
        if (ic_attr->attribute_id == icvalue_id)
            break;
        /*endif*/
    }
    /*endfor*/
    switch (ic_attr->type)
    {
    case XimType_NEST:
        {
            int total_length = 0;
            CARD16 attribute_ID;
            INT16 attribute_length;
            unsigned char *p1 = (unsigned char *) p;
            CARD16 ic_len = 0;
            CARD16 number;
            FrameMgr fm;
            extern XimFrameRec attr_head_fr[];

            while (total_length < value_length)
            {
                fm = FrameMgrInit (attr_head_fr, (char *) p1, need_swap);
                /* get data */
                FrameMgrGetToken (fm, attribute_ID);
                FrameMgrGetToken (fm, attribute_length);
                FrameMgrFree (fm);
                p1 += sizeof (CARD16)*2;
                ReadICValue (i18n_core,
                             attribute_ID,
                             attribute_length,
                             p1,
                             (value_ret + ic_len),
                             &number,
                             need_swap);
                ic_len++;
                *number_ret += number;
                p1 += attribute_length;
                p1 += IMPAD (attribute_length);
                total_length += (CARD16) sizeof(CARD16)*2
                                + (INT16) attribute_length
                                + IMPAD (attribute_length);
            }
	    /*endwhile*/
            return ic_len;
        }

    case XimType_CARD8:
    case XimType_CARD16:
    case XimType_CARD32:
    case XimType_Window:
        SetCardAttribute (value_ret, p, ic_attr, value_length, need_swap);
        *number_ret = (CARD16) 1;
        return *number_ret;

    case XimType_XFontSet:
        SetFontAttribute (value_ret, p, ic_attr, value_length, need_swap);
        *number_ret = (CARD16) 1;
        return *number_ret;

    case XimType_XRectangle:
        SetRectAttribute (value_ret, p, ic_attr, value_length, need_swap);
        *number_ret = (CARD16) 1;
        return *number_ret;

    case XimType_XPoint:
        SetPointAttribute(value_ret, p, ic_attr, value_length, need_swap);
        *number_ret = (CARD16) 1;
        return *number_ret;

#if 0
    case XimType_XIMHotKeyTriggers:
        SetHotKeyAttribute (value_ret, p, ic_attr, value_length, need_swap);
	*number_ret = (CARD16) 1;
	return *number_ret;
#endif
    }
    /*endswitch*/
    return 0;
}

static XICAttribute *CreateNestedList (CARD16 attr_id,
                                       XICAttribute *list,
                                       int number,
                                       int need_swap)
{
    XICAttribute *nest_list = NULL;
    register int i;
    char *values = NULL;
    char *valuesp;
    int value_length = 0;

    if (number == 0)
        return NULL;
    /*endif*/
    for (i = 0;  i < number;  i++)
    {
        value_length += sizeof (CARD16)*2;
        value_length += list[i].value_length;
        value_length += IMPAD (list[i].value_length);
    }
    /*endfor*/
    if ((values = (char *) malloc (value_length)) == NULL)
        return NULL;
    /*endif*/
    memset (values, 0, value_length);

    valuesp = values;
    for (i = 0;  i < number;  i++)
    {
        switch (list[i].type)
        {
        case XimType_CARD8:
        case XimType_CARD16:
        case XimType_CARD32:
        case XimType_Window:
            GetCardAttribute (valuesp, &list[i], need_swap);
            break;

        case XimType_XFontSet:
            GetFontAttribute (valuesp, &list[i], need_swap);
            break;

        case XimType_XRectangle:
            GetRectAttribute (valuesp, &list[i], need_swap);
            break;

        case XimType_XPoint:
            GetPointAttribute (valuesp, &list[i], need_swap);
            break;

#if 0
        case XimType_XIMHotKeyTriggers:
            GetHotKeyAttribute (valuesp, &list[i], need_swap);
            break;
#endif
        }
        /*endswitch*/
        valuesp += sizeof (CARD16)*2;
        valuesp += list[i].value_length;
        valuesp += IMPAD(list[i].value_length);
    }
    /*endfor*/
    
    nest_list = (XICAttribute *) malloc (sizeof (XICAttribute));
    if (nest_list == NULL)
        return NULL;
    /*endif*/
    memset (nest_list, 0, sizeof (XICAttribute));
    nest_list->value = (void *) malloc (value_length);
    if (nest_list->value == NULL)
        return NULL;
    /*endif*/
    memset (nest_list->value, 0, sizeof (value_length));

    nest_list->attribute_id = attr_id;
    nest_list->value_length = value_length;
    memmove (nest_list->value, values, value_length);

    XFree (values);
    return nest_list;
}

static Bool IsNestedList (Xi18n i18n_core, CARD16 icvalue_id)
{
    XICAttr *ic_attr = i18n_core->address.xic_attr;
    int i;

    for (i = 0;  i < i18n_core->address.ic_attr_num;  i++, ic_attr++)
    {
        if (ic_attr->attribute_id == icvalue_id)
        {
            if (ic_attr->type == XimType_NEST)
                return True;
            /*endif*/
            return False;
        }
        /*endif*/
    }
    /*endfor*/
    return False;
}

static Bool IsSeparator (Xi18n i18n_core, CARD16 icvalue_id)
{
    return (i18n_core->address.separatorAttr_id == icvalue_id);
}

static int GetICValue (Xi18n i18n_core,
                       XICAttribute *attr_ret,
                       CARD16 *id_list,
                       int list_num)
{
    XICAttr *xic_attr = i18n_core->address.xic_attr;
    register int i;
    register int j;
    register int n;

    i =
    n = 0;
    if (IsNestedList (i18n_core, id_list[i]))
    {
        i++;
        while (i < list_num  &&  !IsSeparator (i18n_core, id_list[i]))
        {
            for (j = 0;  j < i18n_core->address.ic_attr_num;  j++)
            {
                if (xic_attr[j].attribute_id == id_list[i])
                {
                    attr_ret[n].attribute_id = xic_attr[j].attribute_id;
                    attr_ret[n].name_length = xic_attr[j].length;
                    attr_ret[n].name = malloc (xic_attr[j].length + 1);
		    strcpy(attr_ret[n].name, xic_attr[j].name);
                    attr_ret[n].type = xic_attr[j].type;
                    n++;
                    i++;
                    break;
                }
                /*endif*/
            }
            /*endfor*/
        }
        /*endwhile*/
    }
    else
    {
        for (j = 0;  j < i18n_core->address.ic_attr_num;  j++)
        {
            if (xic_attr[j].attribute_id == id_list[i])
            {
                attr_ret[n].attribute_id = xic_attr[j].attribute_id;
                attr_ret[n].name_length = xic_attr[j].length;
                attr_ret[n].name = malloc (xic_attr[j].length + 1);
		strcpy(attr_ret[n].name, xic_attr[j].name);
                attr_ret[n].type = xic_attr[j].type;
                n++;
                break;
            }
            /*endif*/
        }
        /*endfor*/
    }
    /*endif*/
    return n;
}

/* called from CreateICMessageProc and SetICValueMessageProc */
void _Xi18nChangeIC (XIMS ims,
                     IMProtocol *call_data,
                     unsigned char *p,
                     int create_flag)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    FmStatus status;
    CARD16 byte_length;
    register int total_size;
    unsigned char *reply = NULL;
    register int i;
    register int attrib_num;
    XICAttribute *attrib_list;
    XICAttribute pre_attr[IC_SIZE];
    XICAttribute sts_attr[IC_SIZE];
    XICAttribute ic_attr[IC_SIZE];
    CARD16 preedit_ic_num = 0;
    CARD16 status_ic_num = 0;
    CARD16 ic_num = 0;
    CARD16 connect_id = call_data->any.connect_id;
    IMChangeICStruct *changeic = (IMChangeICStruct *) &call_data->changeic;
    extern XimFrameRec create_ic_fr[];
    extern XimFrameRec create_ic_reply_fr[];
    extern XimFrameRec set_ic_values_fr[];
    extern XimFrameRec set_ic_values_reply_fr[];
    CARD16 input_method_ID;

    memset (pre_attr, 0, sizeof (XICAttribute)*IC_SIZE);
    memset (sts_attr, 0, sizeof (XICAttribute)*IC_SIZE);
    memset (ic_attr, 0, sizeof (XICAttribute)*IC_SIZE);

    if (create_flag == True)
    {
        fm = FrameMgrInit (create_ic_fr,
                           (char *) p,
                           _Xi18nNeedSwap (i18n_core, connect_id));
        /* get data */
        FrameMgrGetToken (fm, input_method_ID);
        FrameMgrGetToken (fm, byte_length);
    }
    else
    {
        fm = FrameMgrInit (set_ic_values_fr,
                           (char *) p,
                           _Xi18nNeedSwap (i18n_core, connect_id));
        /* get data */
        FrameMgrGetToken (fm, input_method_ID);
        FrameMgrGetToken (fm, changeic->icid);
        FrameMgrGetToken (fm, byte_length);
    }
    /*endif*/
    attrib_list = (XICAttribute *) malloc (sizeof (XICAttribute)*IC_SIZE);
    if (!attrib_list)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (attrib_list, 0, sizeof(XICAttribute)*IC_SIZE);

    attrib_num = 0;
    while (FrameMgrIsIterLoopEnd (fm, &status) == False)
    {
        void *value;
        int value_length;
        
        FrameMgrGetToken (fm, attrib_list[attrib_num].attribute_id);
        FrameMgrGetToken (fm, value_length);
        FrameMgrSetSize (fm, value_length);
        attrib_list[attrib_num].value_length = value_length;
        FrameMgrGetToken (fm, value);
        attrib_list[attrib_num].value = (void *) malloc (value_length + 1);
        memmove (attrib_list[attrib_num].value, value, value_length);
	((char *)attrib_list[attrib_num].value)[value_length] = '\0';
        attrib_num++;
    }
    /*endwhile*/

    for (i = 0;  i < attrib_num;  i++)
    {
        CARD16 number;
        
        if (IsNestedList (i18n_core, attrib_list[i].attribute_id))
        {
            if (attrib_list[i].attribute_id
                == i18n_core->address.preeditAttr_id)
            {
                ReadICValue (i18n_core,
                             attrib_list[i].attribute_id,
                             attrib_list[i].value_length,
                             attrib_list[i].value,
                             &pre_attr[preedit_ic_num],
                             &number,
                             _Xi18nNeedSwap(i18n_core, connect_id));
                preedit_ic_num += number;
            }
            else if (attrib_list[i].attribute_id == i18n_core->address.statusAttr_id)
            {
                ReadICValue (i18n_core,
                             attrib_list[i].attribute_id,
                             attrib_list[i].value_length,
                             attrib_list[i].value,
                             &sts_attr[status_ic_num],
                             &number,
                             _Xi18nNeedSwap (i18n_core, connect_id));
                status_ic_num += number;
            }
            else
            {
                /* another nested list.. possible? */
            }
            /*endif*/
        }
        else
        {
            ReadICValue (i18n_core,
                         attrib_list[i].attribute_id,
                         attrib_list[i].value_length,
                         attrib_list[i].value,
                         &ic_attr[ic_num],
                         &number,
                         _Xi18nNeedSwap (i18n_core, connect_id));
            ic_num += number;
        }
        /*endif*/
    }
    /*endfor*/
    for (i = 0;  i < attrib_num;  i++)
        XFree (attrib_list[i].value);
    /*endfor*/
    XFree (attrib_list);

    FrameMgrFree (fm);

    changeic->preedit_attr_num = preedit_ic_num;
    changeic->status_attr_num = status_ic_num;
    changeic->ic_attr_num = ic_num;
    changeic->preedit_attr = pre_attr;
    changeic->status_attr = sts_attr;
    changeic->ic_attr = ic_attr;

    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto(ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
    if (create_flag == True)
    {
        fm = FrameMgrInit (create_ic_reply_fr,
                           NULL,
                           _Xi18nNeedSwap (i18n_core, connect_id));
    }
    else
    {
        fm = FrameMgrInit (set_ic_values_reply_fr,
                           NULL,
                           _Xi18nNeedSwap (i18n_core, connect_id));
    }
    /*endif*/
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
    FrameMgrPutToken (fm, changeic->icid);

    if (create_flag == True)
    {
        _Xi18nSendMessage (ims,
                           connect_id,
                           XIM_CREATE_IC_REPLY,
                           0,
                           reply,
                           total_size);
    }
    else
    {
        _Xi18nSendMessage (ims,
                           connect_id,
                           XIM_SET_IC_VALUES_REPLY,
                           0,
                           reply,
                           total_size);
    }
    /*endif*/
    if (create_flag == True)
    {
        int on_key_num = i18n_core->address.on_keys.count_keys;
        int off_key_num = i18n_core->address.off_keys.count_keys;

        if (on_key_num == 0  &&  off_key_num == 0)
        {
            long mask;

            if (i18n_core->address.imvalue_mask & I18N_FILTERMASK)
                mask = i18n_core->address.filterevent_mask;
            else
                mask = DEFAULT_FILTER_MASK;
            /*endif*/
            /* static event flow is default */
            _Xi18nSetEventMask (ims,
                                connect_id,
                                input_method_ID,
                                changeic->icid,
                                mask,
                                ~mask);
        }
        /*endif*/
    }
    /*endif*/
    FrameMgrFree (fm);
    XFree(reply);
}

/* called from GetICValueMessageProc */
void _Xi18nGetIC (XIMS ims, IMProtocol *call_data, unsigned char *p)
{
    Xi18n i18n_core = ims->protocol;
    FrameMgr fm;
    FmStatus status;
    extern XimFrameRec get_ic_values_fr[];
    extern XimFrameRec get_ic_values_reply_fr[];
    CARD16 byte_length;
    register int total_size;
    unsigned char *reply = NULL;
    XICAttribute *preedit_ret = NULL;
    XICAttribute *status_ret = NULL;
    register int i;
    register int number;
    int iter_count;
    CARD16 *attrID_list;
    XICAttribute pre_attr[IC_SIZE];
    XICAttribute sts_attr[IC_SIZE];
    XICAttribute ic_attr[IC_SIZE];
    CARD16 pre_count = 0;
    CARD16 sts_count = 0;
    CARD16 ic_count = 0;
    IMChangeICStruct *getic = (IMChangeICStruct *) &call_data->changeic;
    CARD16 connect_id = call_data->any.connect_id;
    CARD16 input_method_ID;

    memset (pre_attr, 0, sizeof (XICAttribute)*IC_SIZE);
    memset (sts_attr, 0, sizeof (XICAttribute)*IC_SIZE);
    memset (ic_attr, 0, sizeof (XICAttribute)*IC_SIZE);

    fm = FrameMgrInit (get_ic_values_fr,
                       (char *) p,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* get data */
    FrameMgrGetToken (fm, input_method_ID);
    FrameMgrGetToken (fm, getic->icid);
    FrameMgrGetToken (fm, byte_length);

    attrID_list = (CARD16 *) malloc (sizeof (CARD16)*IC_SIZE);  /* bogus */
    memset (attrID_list, 0, sizeof (CARD16)*IC_SIZE);

    number = 0;
    while (FrameMgrIsIterLoopEnd (fm, &status) == False)
        FrameMgrGetToken (fm, attrID_list[number++]);
    /*endwhile*/
    FrameMgrFree (fm);

    i = 0;
    while (i < number)
    {
        int read_number;
        
        if (IsNestedList (i18n_core, attrID_list[i]))
        {
            if (attrID_list[i] == i18n_core->address.preeditAttr_id)
            {
                read_number = GetICValue (i18n_core,
                                          &pre_attr[pre_count],
                                          &attrID_list[i],
                                          number);
                i += read_number + 1;
                pre_count += read_number;
            }
            else if (attrID_list[i] == i18n_core->address.statusAttr_id)
            {
                read_number = GetICValue (i18n_core,
                                          &sts_attr[sts_count],
                                          &attrID_list[i],
                                          number);
                i += read_number + 1;
                sts_count += read_number;
            }
            else
            {
                /* another nested list.. possible? */
            }
            /*endif*/
        }
        else
        {
            read_number = GetICValue (i18n_core,
                                      &ic_attr[ic_count],
                                      &attrID_list[i],
                                      number);
            i += read_number;
            ic_count += read_number;
        }
        /*endif*/
    }
    /*endwhile*/
    getic->preedit_attr_num = pre_count;
    getic->status_attr_num = sts_count;
    getic->ic_attr_num = ic_count;
    getic->preedit_attr = pre_attr;
    getic->status_attr = sts_attr;
    getic->ic_attr = ic_attr;
    if (i18n_core->address.improto)
    {
        if (!(i18n_core->address.improto (ims, call_data)))
            return;
        /*endif*/
    }
    /*endif*/
    iter_count = getic->ic_attr_num;

    preedit_ret = CreateNestedList (i18n_core->address.preeditAttr_id,
                                    getic->preedit_attr,
                                    getic->preedit_attr_num,
                                    _Xi18nNeedSwap (i18n_core, connect_id));
    if (preedit_ret)
        iter_count++;
    /*endif*/
    status_ret = CreateNestedList (i18n_core->address.statusAttr_id,
                                   getic->status_attr,
                                   getic->status_attr_num,
                                   _Xi18nNeedSwap (i18n_core, connect_id));
    if (status_ret)
        iter_count++;
    /*endif*/

    fm = FrameMgrInit (get_ic_values_reply_fr,
                       NULL,
                       _Xi18nNeedSwap (i18n_core, connect_id));

    /* set iteration count for list of ic_attribute */
    FrameMgrSetIterCount (fm, iter_count);

    /* set length of BARRAY item in xicattribute_fr */
    for (i = 0;  i < (int) getic->ic_attr_num;  i++)
        FrameMgrSetSize (fm, ic_attr[i].value_length);
    /*endfor*/
    
    if (preedit_ret)
        FrameMgrSetSize (fm, preedit_ret->value_length);
    /*endif*/
    if (status_ret)
        FrameMgrSetSize (fm, status_ret->value_length);
    /*endif*/
    total_size = FrameMgrGetTotalSize (fm);
    reply = (unsigned char *) malloc (total_size);
    if (reply == NULL)
    {
        _Xi18nSendMessage (ims, connect_id, XIM_ERROR, 0, 0, 0);
        return;
    }
    /*endif*/
    memset (reply, 0, total_size);
    FrameMgrSetBuffer (fm, reply);

    FrameMgrPutToken (fm, input_method_ID);
    FrameMgrPutToken (fm, getic->icid);

    for (i = 0;  i < (int) getic->ic_attr_num;  i++)
    {
        FrameMgrPutToken (fm, ic_attr[i].attribute_id);
        FrameMgrPutToken (fm, ic_attr[i].value_length);
        FrameMgrPutToken (fm, ic_attr[i].value);
    }
    /*endfor*/
    if (preedit_ret)
    {
        FrameMgrPutToken (fm, preedit_ret->attribute_id);
        FrameMgrPutToken (fm, preedit_ret->value_length);
        FrameMgrPutToken (fm, preedit_ret->value);
    }
    /*endif*/
    if (status_ret)
    {
        FrameMgrPutToken (fm, status_ret->attribute_id);
        FrameMgrPutToken (fm, status_ret->value_length);
        FrameMgrPutToken (fm, status_ret->value);
    }
    /*endif*/
    _Xi18nSendMessage (ims,
                       connect_id,
                       XIM_GET_IC_VALUES_REPLY,
                       0,
                       reply,
                       total_size);
    XFree (reply);
    XFree (attrID_list);

    for (i = 0;  i < (int) getic->ic_attr_num;  i++)
    {
	if (getic->ic_attr[i].name)
	    XFree (getic->ic_attr[i].name);
	/*endif*/
        if (getic->ic_attr[i].value)
            XFree (getic->ic_attr[i].value);
        /*endif*/
    }
    /*endfor*/
    for (i = 0;  i < (int) getic->preedit_attr_num;  i++)
    {
	if (getic->preedit_attr[i].name)
	    XFree (getic->preedit_attr[i].name);
	/*endif*/
	if (getic->preedit_attr[i].value)
	    XFree (getic->preedit_attr[i].value);
	/*endif*/
    }
    /*endfor*/
    for (i = 0;  i < (int) getic->status_attr_num;  i++)
    {
	if (getic->status_attr[i].name)
	    XFree (getic->status_attr[i].name);
	/*endif*/
	if (getic->status_attr[i].value)
	    XFree (getic->status_attr[i].value);
	/*endif*/
    }
    /*endfor*/
    
    if (preedit_ret)
    {
        XFree (preedit_ret->value);
        XFree (preedit_ret);
    }
    /*endif*/
    if (status_ret)
    {
        XFree (status_ret->value);
        XFree (status_ret);
    }
    /*endif*/
    FrameMgrFree (fm);
}
