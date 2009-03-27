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

#ifndef _Xi18n_h
#define _Xi18n_h
#include <X11/Xlib.h>
#include <X11/Xfuncs.h>
#include <X11/Xos.h>
#include "XimProto.h"

/*
 * Minor Protocol Number for Extension Protocol 
 */
#define XIM_EXTENSION				128
#define XIM_EXT_SET_EVENT_MASK			(0x30)
#define	XIM_EXT_FORWARD_KEYEVENT		(0x32)
#define	XIM_EXT_MOVE				(0x33)
#define COMMON_EXTENSIONS_NUM   		3

#include <stdlib.h>
#include "IMdkit.h"

/* XI18N Valid Attribute Name Definition */
#define ExtForwardKeyEvent	"extForwardKeyEvent"
#define ExtMove			"extMove"
#define ExtSetEventMask		"extSetEventMask"

/*
 * Padding macro
 */
#define	IMPAD(length) ((4 - ((length)%4))%4)

/*
 * Target Atom for Transport Connection
 */
#define LOCALES		"LOCALES"
#define TRANSPORT	"TRANSPORT"

#define I18N_OPEN	0
#define I18N_SET	1
#define I18N_GET	2

typedef struct
{
    char        *transportname;
    int         namelen;
    Bool        (*checkAddr) ();
} TransportSW;

typedef struct _XIMPending
{
    unsigned    char *p;
    struct _XIMPending *next;
} XIMPending;

typedef struct _XimProtoHdr
{
    CARD8	major_opcode;
    CARD8	minor_opcode;
    CARD16	length;
} XimProtoHdr;

typedef struct
{
    CARD16	attribute_id;
    CARD16	type;
    CARD16	length;
    char	*name;
} XIMAttr;

typedef struct
{
    CARD16	attribute_id;
    CARD16	type;
    CARD16	length;
    char	*name;
} XICAttr;

typedef struct
{
    int		attribute_id;
    CARD16	name_length; 
    char	*name;
    int		value_length;
    void	*value;
    int		type;
} XIMAttribute;

typedef struct
{
    int		attribute_id;
    CARD16	name_length;
    char	*name;
    int		value_length;
    void	*value;
    int		type;
} XICAttribute;

typedef struct
{
    int		length;
    char	*name;
} XIMStr;

typedef struct
{
    CARD16	major_opcode;
    CARD16	minor_opcode;
    CARD16	length;
    char	*name;
} XIMExt;

typedef struct _Xi18nClient
{
    int		connect_id;
    CARD8	byte_order;
    /*
       '?': initial value
       'B': for Big-Endian
       'l': for little-endian
     */
    int		sync;
    XIMPending  *pending;
    void *trans_rec;		/* contains transport specific data  */
    struct _Xi18nClient *next;
} Xi18nClient;

typedef struct _Xi18nCore *Xi18n;

/*
 * Callback Struct for XIM Protocol
 */
typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
} IMAnyStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD8	byte_order;
    CARD16	major_version;
    CARD16	minor_version;
} IMConnectStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
} IMDisConnectStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    XIMStr	lang;
} IMOpenStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
} IMCloseStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	number;
    XIMStr	*extension;
} IMQueryExtensionStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	number;
    char	**im_attr_list;
} IMGetIMValuesStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    CARD16	preedit_attr_num;
    CARD16	status_attr_num;
    CARD16	ic_attr_num;
    XICAttribute *preedit_attr;
    XICAttribute *status_attr;
    XICAttribute *ic_attr;
} IMChangeICStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
} IMDestroyICStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    CARD16	length;
    char	*commit_string;
} IMResetICStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
} IMChangeFocusStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    BITMASK16	sync_bit;
    CARD16	serial_number;
    XEvent	event;
} IMForwardEventStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    CARD16	flag;
    KeySym	keysym;
    char	*commit_string;
} IMCommitStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    CARD32	flag;
    CARD32	key_index;
    CARD32	event_mask;
} IMTriggerNotifyStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	encoding_number;
    XIMStr	*encoding;	/* name information */
    CARD16	encoding_info_number;
    XIMStr	*encodinginfo;	/* detailed information */
    CARD16	category;	/* #0 for name, #1 for detail */
    INT16	enc_index;	/* index of the encoding determined */
} IMEncodingNegotiationStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    CARD32	flag;
    CARD32	forward_event_mask;
    CARD32	sync_event_mask;
} IMSetEventMaskStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    CARD32	filter_event_mask;
    CARD32	intercept_event_mask;
    CARD32	select_event_mask;
    CARD32	forward_event_mask;
    CARD32	sync_event_mask;
} IMExtSetEventMaskStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    CARD16	x;
    CARD16	y;
} IMMoveStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    BITMASK16	flag;
    CARD16	error_code;
    CARD16	str_length;
    CARD16	error_type;
    char	*error_detail;
} IMErrorStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
} IMPreeditStateStruct;

/* Callbacks */
typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
} IMGeometryCBStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    union
    {
	int return_value;			/* PreeditStart */
	XIMPreeditDrawCallbackStruct draw;	/* PreeditDraw */
	XIMPreeditCaretCallbackStruct caret; 	/* PreeditCaret */
    } todo;
} IMPreeditCBStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    union
    {
	XIMStatusDrawCallbackStruct draw;	/* StatusDraw */
    } todo;
} IMStatusCBStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
    XIMStringConversionCallbackStruct strconv;
} IMStrConvCBStruct;

typedef struct
{
    int		major_code;
    int		minor_code;
    CARD16	connect_id;
    CARD16	icid;
} IMSyncXlibStruct;

typedef union _IMProtocol
{
    int	major_code;
    IMAnyStruct any;
    IMConnectStruct imconnect;
    IMDisConnectStruct imdisconnect;
    IMOpenStruct imopen;
    IMCloseStruct imclose;
    IMQueryExtensionStruct queryext;
    IMGetIMValuesStruct getim;
    IMEncodingNegotiationStruct encodingnego;
    IMExtSetEventMaskStruct extsetevent;
    IMMoveStruct extmove;
    IMSetEventMaskStruct setevent;
    IMChangeICStruct changeic;
    IMDestroyICStruct destroyic;
    IMResetICStruct resetic;
    IMChangeFocusStruct changefocus;
    IMCommitStruct commitstring;
    IMForwardEventStruct forwardevent;
    IMTriggerNotifyStruct triggernotify;
    IMPreeditStateStruct preedit_state;
    IMErrorStruct imerror;
    IMGeometryCBStruct geometry_callback;
    IMPreeditCBStruct preedit_callback;
    IMStatusCBStruct status_callback;
    IMStrConvCBStruct strconv_callback;
    IMSyncXlibStruct sync_xlib;
    long pad[32];
} IMProtocol;

typedef int (*IMProtoHandler) (XIMS, IMProtocol*);

#define DEFAULT_FILTER_MASK	(KeyPressMask)

/* Xi18nAddressRec structure */
typedef struct _Xi18nAddressRec
{
    Display	*dpy;
    CARD8	im_byteOrder;	/* byte order 'B' or 'l' */
    /* IM Values */
    long	imvalue_mask;
    Window	im_window;	/* IMServerWindow */
    char	*im_name;	/* IMServerName */
    char	*im_locale;	/* IMLocale */
    char	*im_addr;	/* IMServerTransport */
    XIMStyles	input_styles;	/* IMInputStyles */
    XIMTriggerKeys on_keys;	/* IMOnKeysList */
    XIMTriggerKeys off_keys;	/* IMOffKeysList */
    XIMEncodings encoding_list; /* IMEncodingList */
    IMProtoHandler improto;	/* IMProtocolHander */
    long	filterevent_mask; /* IMFilterEventMask */
    /* XIM_SERVERS target Atoms */
    Atom	selection;
    Atom	Localename;
    Atom	Transportname;
    /* XIM/XIC Attr */
    int		im_attr_num;
    XIMAttr	*xim_attr;
    int		ic_attr_num;
    XICAttr	*xic_attr;
    CARD16	preeditAttr_id;
    CARD16	statusAttr_id;
    CARD16	separatorAttr_id;
    /* XIMExtension List */
    int		ext_num;
    XIMExt	extension[COMMON_EXTENSIONS_NUM];
    /* transport specific connection address */
    void	*connect_addr;
    /* actual data is defined:
       XSpecRec in Xi18nX.h for X-based connection.
       TransSpecRec in Xi18nTr.h for Socket-based connection.
     */
    /* clients table */
    Xi18nClient *clients;
    Xi18nClient *free_clients;
} Xi18nAddressRec;

typedef struct _Xi18nMethodsRec
{
    Bool (*begin) (XIMS);
    Bool (*end) (XIMS);
    Bool (*send) (XIMS, CARD16, unsigned char*, long);
    Bool (*wait) (XIMS, CARD16, CARD8, CARD8);
    Bool (*disconnect) (XIMS, CARD16);
} Xi18nMethodsRec;

typedef struct _Xi18nCore
{
    Xi18nAddressRec address;
    Xi18nMethodsRec methods;
} Xi18nCore;

#endif

