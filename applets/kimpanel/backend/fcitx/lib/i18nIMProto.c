/******************************************************************
Copyright 1993, 1994 by Digital Equipment Corporation, Maynard, Massachusetts,
Copyright 1993, 1994 by Hewlett-Packard Company
 
Copyright 1994, 1995 by Sun Microsystems, Inc.
 
                        All Rights Reserved
 
Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Digital or MIT not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  
 
DIGITAL AND HEWLETT-PACKARD COMPANY DISCLAIMS ALL WARRANTIES WITH REGARD
TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS, IN NO EVENT SHALL DIGITAL AND HEWLETT-PACKARD COMPANY BE LIABLE
FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
  Author: Hiroyuki Miyamoto  Digital Equipment Corporation
                             miyamoto@jrd.dec.com
	  Hidetoshi Tajima(tajima@Eng.Sun.COM) Sun Microsystems, Inc.

    This version tidied and debugged by Steve Underwood May 1999
 
******************************************************************/

/* Protocol Packet frames */

#include "FrameMgr.h"

/* Data type definitions */

static XimFrameRec ximattr_fr[] =
{
    _FRAME(BIT16), 		/* attribute ID */
    _FRAME(BIT16), 		/* type of the value */
    _FRAME(BIT16), 		/* length of im-attribute */
    _FRAME(BARRAY), 		/* im-attribute */
    _PAD4(2),
    _FRAME(EOL),
};

static XimFrameRec xicattr_fr[] =
{
    _FRAME(BIT16), 		/* attribute ID */
    _FRAME(BIT16), 		/* type of the value */
    _FRAME(BIT16), 		/* length of ic-attribute */
    _FRAME(BARRAY), 		/* ic-attribute */
    _PAD4(2),
    _FRAME(EOL),
};

static XimFrameRec ximattribute_fr[] =
{
    _FRAME(BIT16), 		/* attribute ID */
    _FRAME(BIT16), 		/* value length */
    _FRAME(BARRAY),             /* value */
    _PAD4(1),
    _FRAME(EOL),
};

static XimFrameRec xicattribute_fr[] =
{
    _FRAME(BIT16), 		/* attribute ID */
    _FRAME(BIT16), 		/* value length */
    _FRAME(BARRAY),             /* value */
    _PAD4(1),
    _FRAME(EOL),
};

static XimFrameRec ximtriggerkey_fr[] =
{
    _FRAME(BIT32), 		/* keysym */
    _FRAME(BIT32), 		/* modifier */
    _FRAME(BIT32), 		/* modifier mask */
    _FRAME(EOL),
};

static XimFrameRec encodinginfo_fr[] =
{
    _FRAME(BIT16), 		/* length of encoding info */
    _FRAME(BARRAY), 		/* encoding info */
    _PAD4(2),
    _FRAME(EOL),
};

static XimFrameRec str_fr[] =
{
    _FRAME(BIT8), 		/* number of byte */
    _FRAME(BARRAY), 		/* string */
    _FRAME(EOL),
};

static XimFrameRec xpcs_fr[] =
{
    _FRAME(BIT16), 		/* length of string in bytes */
    _FRAME(BARRAY), 		/* string */
    _PAD4(2),
};

static XimFrameRec ext_fr[] =
{
    _FRAME(BIT16), 		/* extension major-opcode */
    _FRAME(BIT16), 		/* extension minor-opcode */
    _FRAME(BIT16), 		/* length of extension name */
    _FRAME(BARRAY), 		/* extension name */
    _PAD4(2),
    _FRAME(EOL),
};

static XimFrameRec inputstyle_fr[] =
{
    _FRAME(BIT32), 		/* inputstyle */
    _FRAME(EOL),
};
/* Protocol definitions */

xim_externaldef XimFrameRec attr_head_fr[] =
{
    _FRAME(BIT16), 	/* attribute id */
    _FRAME(BIT16), 	/* attribute length */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec short_fr[] =
{
    _FRAME(BIT16), 	/* value */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec long_fr[] =
{
    _FRAME(BIT32), 	/* value */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec xrectangle_fr[] =
{
    _FRAME(BIT16), 	/* x */
    _FRAME(BIT16), 	/* y */
    _FRAME(BIT16), 	/* width */
    _FRAME(BIT16), 	/* height */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec xpoint_fr[] =
{
    _FRAME(BIT16), 	/* x */
    _FRAME(BIT16), 	/* y */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec fontset_fr[] =
{
    _FRAME(BIT16), 	/* length of base font name */
    _FRAME(BARRAY), 	/* base font name list */
    _PAD4(2), 		/* unused */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec input_styles_fr[] =
{
    _FRAME(BIT16), 		/* number of list */
    _PAD4(1), 			/* unused */
    _FRAME(ITER), 		/* XIMStyle list */
    _FRAME(POINTER),
    _PTR(inputstyle_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec packet_header_fr[] =
{
    _FRAME(BIT8), 		/* major-opcode */
    _FRAME(BIT8), 		/* minor-opcode */
    _FRAME(BIT16), 		/* length */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec error_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* flag */
    _FRAME(BIT16), 		/* Error Code */
    _FRAME(BIT16), 		/* length of error detail */
    _FRAME(BIT16), 		/* type of error detail */
    _FRAME(BARRAY), 		/* error detail */
    _PAD4(1),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec connect_fr[] =
{
    _FRAME(BIT8), 		/* byte order */
    _PAD2(1), 			/* unused */
    _FRAME(BIT16), 		/* client-major-protocol-version */
    _FRAME(BIT16), 		/* client-minor-protocol-version */
    _BYTE_COUNTER(BIT16, 1), 	/* length of client-auth-protocol-names */
    _FRAME(ITER), 		/* client-auth-protocol-names */
    _FRAME(POINTER),
    _PTR(xpcs_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec connect_reply_fr[] =
{
    _FRAME(BIT16), 		/* server-major-protocol-version */
    _FRAME(BIT16), 		/* server-minor-protocol-version */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec auth_required_fr[] =
{
    _FRAME(BIT8), 		/* auth-protocol-index */
    _FRAME(BIT8), 		/* auth-data1 */
    _FRAME(BARRAY), 		/* auth-data2 */
    _PAD4(3),
    _FRAME(EOL),
};


xim_externaldef XimFrameRec auth_reply_fr[] =
{
    _FRAME(BIT8),
    _FRAME(BARRAY),
    _PAD4(2),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec auth_next_fr[] =
{
    _FRAME(BIT8), 		/* auth-data1 */
    _FRAME(BARRAY), 		/* auth-data2 */
    _PAD4(2),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec auth_setup_fr[] =
{
    _BYTE_COUNTER(BIT16, 2), 	/* number of client-auth-protocol-names */
    _PAD4(1), 			/* unused */
    _FRAME(ITER), 		/* server-auth-protocol-names */
    _FRAME(POINTER),
    _PTR(xpcs_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec auth_ng_fr[] =
{
    _FRAME(EOL),
};

xim_externaldef XimFrameRec disconnect_fr[] =
{
    _FRAME(EOL),
};

xim_externaldef XimFrameRec disconnect_reply_fr[] =
{
    _FRAME(EOL),
};

xim_externaldef XimFrameRec open_fr[] =
{
    _FRAME(POINTER), 		/* locale name */
    _PTR(str_fr),
    _PAD4(1),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec open_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of IM attributes supported */
    _FRAME(ITER), 		/* IM attribute supported */
    _FRAME(POINTER),
    _PTR(ximattr_fr),
    _BYTE_COUNTER(BIT16, 2), 	/* number of IC attribute supported */
    _PAD4(1), 			/* unused */
    _FRAME(ITER), 		/* IC attribute supported */
    _FRAME(POINTER),
    _PTR(xicattr_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec close_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _PAD4(1), 			/* unused */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec close_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _PAD4(1), 			/* unused */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec register_triggerkeys_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _PAD4(1), 			/* unused */
    _BYTE_COUNTER(BIT32, 1),    /* byte length of on-keys */
    _FRAME(ITER), 		/* on-keys list */
    _FRAME(POINTER),
    _PTR(ximtriggerkey_fr),
    _BYTE_COUNTER(BIT32, 1), 	/* byte length of off-keys */
    _FRAME(ITER), 		/* off-keys list */
    _FRAME(POINTER),
    _PTR(ximtriggerkey_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec trigger_notify_fr[] =
{
    _FRAME(BIT16), 		/* input-mehotd-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* flag */
    _FRAME(BIT32), 		/* index of keys list */
    _FRAME(BIT32), 		/* client-select-event-mask */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec trigger_notify_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec set_event_mask_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* forward-event-mask */
    _FRAME(BIT32), 		/* synchronous-event-mask */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec encoding_negotiation_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of encodings listed by name */
    _FRAME(ITER), 		/* supported list of encoding in IM library */
    _FRAME(POINTER),
    _PTR(str_fr),
    _PAD4(1),
    _BYTE_COUNTER(BIT16, 2), 	/* byte length of encodings listed by
                                       detailed data */
    _PAD4(1),
    _FRAME(ITER), 		/* list of encodings supported in the
    				   IM library */
    _FRAME(POINTER),
    _PTR(encodinginfo_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec encoding_negotiation_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* category of the encoding determined */
    _FRAME(BIT16), 		/* index of the encoding dterminated */
    _PAD4(1),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec query_extension_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of extensions supported
    				   by the IM library */
    _FRAME(ITER), 		/* extensions supported by the IM library */
    _FRAME(POINTER),
    _PTR(str_fr),
    _PAD4(1),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec query_extension_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of extensions supported
    				   by the IM server */
    _FRAME(ITER), 		/* list of extensions supported by the
    				   IM server */
    _FRAME(POINTER),
    _PTR(ext_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec get_im_values_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of im-attribute-id */
    _FRAME(ITER), 		/* im-attribute-id */
    _FRAME(BIT16),
    _PAD4(1),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec get_im_values_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of im-attribute returned */
    _FRAME(ITER), 		/* im-attribute returned */
    _FRAME(POINTER),
    _PTR(ximattribute_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec create_ic_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of ic-attributes */
    _FRAME(ITER), 		/* ic-attributes */
    _FRAME(POINTER),
    _PTR(xicattribute_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec create_ic_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec destroy_ic_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec destroy_ic_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec set_ic_values_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _BYTE_COUNTER(BIT16, 2), 	/* byte length of ic-attributes */
    _PAD4(1),
    _FRAME(ITER), 		/* ic-attribute */
    _FRAME(POINTER),
    _PTR(xicattribute_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec set_ic_values_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec get_ic_values_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of ic-attribute-id */
    _FRAME(ITER), 		/* ic-attribute */
    _FRAME(BIT16),
    _PAD4(2),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec get_ic_values_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _BYTE_COUNTER(BIT16, 2), 	/* byte length of ic-attribute */
    _PAD4(1),
    _FRAME(ITER), 		/* ic-attribute */
    _FRAME(POINTER),
    _PTR(xicattribute_fr),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec set_ic_focus_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec unset_ic_focus_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec forward_event_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* flag */
    _FRAME(BIT16), 		/* sequence number */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec sync_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec sync_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

#if 0
xim_externaldef XimFrameRec commit_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* flag */
    _FRAME(BIT16), 		/* byte length of committed string */
    _FRAME(BARRAY), 		/* committed string */
    _PAD4(1),
    _BYTE_COUNTER(BIT16, 1), 	/* byte length of keysym */
    _FRAME(ITER), 		/* keysym */
    _FRAME(BIT32),
    _PAD4(1),
    _FRAME(EOL),
};
#endif

xim_externaldef XimFrameRec commit_chars_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* flag */
    _FRAME(BIT16), 		/* byte length of committed string */
    _FRAME(BARRAY), 		/* committed string */
    _PAD4(1),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec commit_both_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* flag */
    _PAD4(1), 			/* unused */
    _FRAME(BIT32), 		/* keysym */
    _FRAME(BIT16), 		/* byte length of committed string */
    _FRAME(BARRAY), 		/* committed string */
    _PAD4(2),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec reset_ic_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec reset_ic_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* byte length of committed string */
    _FRAME(BARRAY), 		/* committed string */
    _PAD4(2),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec geometry_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec str_conversion_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* XIMStringConversionPosition */
    _FRAME(BIT32), 		/* XIMStringConversionType */
    _FRAME(BIT32), 		/* XIMStringConversionOperation */
    _FRAME(BIT16), 		/* length to multiply the
    				   XIMStringConversionType */
    _FRAME(BIT16), 		/* length of the string to be
    				   substituted */
#if 0
    _FRAME(BARRAY), 		/* string */
    _PAD4(1),
#endif
    _FRAME(EOL),
};

xim_externaldef XimFrameRec str_conversion_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* XIMStringConversionFeedback */
    _FRAME(BIT16), 		/* length of the retrieved string */
    _FRAME(BARRAY), 		/* retrieved string */
    _PAD4(2),
    _BYTE_COUNTER(BIT16, 2), 	/* number of feedback array */
    _PAD4(1),
    _FRAME(ITER), 		/* feedback array */
    _FRAME(BIT32),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec preedit_start_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec preedit_start_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* return value */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec preedit_draw_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* caret */
    _FRAME(BIT32), 		/* chg_first */
    _FRAME(BIT32), 		/* chg_length */
    _FRAME(BIT32), 		/* status */
    _FRAME(BIT16), 		/* length of preedit string */
    _FRAME(BARRAY), 		/* preedit string */
    _PAD4(2),
    _BYTE_COUNTER(BIT16, 2), 	/* number of feedback array */
    _PAD4(1),
    _FRAME(ITER), 		/* feedback array */
    _FRAME(BIT32),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec preedit_caret_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* position */
    _FRAME(BIT32), 		/* direction */
    _FRAME(BIT32), 		/* style */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec preedit_caret_reply_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* position */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec preedit_done_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec status_start_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec status_draw_text_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* type */
    _FRAME(BIT32), 		/* status */
    _FRAME(BIT16), 		/* length of status string */
    _FRAME(BARRAY), 		/* status string */
    _PAD4(2),
    _BYTE_COUNTER(BIT16, 2), 	/* number of feedback array */
    _PAD4(1),
    _FRAME(ITER), 		/* feedback array */
    _FRAME(BIT32),
    _FRAME(EOL),
};

xim_externaldef XimFrameRec status_draw_bitmap_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* type */
    _FRAME(BIT32), 		/* pixmap data */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec status_done_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec ext_set_event_mask_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT32), 		/* filter-event-mask */
    _FRAME(BIT32), 		/* intercept-event-mask */
    _FRAME(BIT32), 		/* select-event-mask */
    _FRAME(BIT32), 		/* forward-event-mask */
    _FRAME(BIT32), 		/* synchronous-event-mask */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec ext_forward_keyevent_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* flag */
    _FRAME(BIT16), 		/* sequence number */
    _FRAME(BIT8), 		/* xEvent.u.u.type */
    _FRAME(BIT8), 		/* keycode */
    _FRAME(BIT16), 		/* state */
    _FRAME(BIT32), 		/* time */
    _FRAME(BIT32), 		/* window */
    _FRAME(EOL),
};

xim_externaldef XimFrameRec ext_move_fr[] =
{
    _FRAME(BIT16), 		/* input-method-ID */
    _FRAME(BIT16), 		/* input-context-ID */
    _FRAME(BIT16), 		/* X */
    _FRAME(BIT16), 		/* Y */
    _FRAME(EOL),
};
