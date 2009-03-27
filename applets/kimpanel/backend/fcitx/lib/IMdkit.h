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

#ifndef _IMdkit_h
#define _IMdkit_h

#include <X11/Xmd.h>

#ifdef __cplusplus
extern "C" {
#endif

/* IM Attributes Name */
#define IMModifiers		"modifiers"
#define IMServerWindow		"serverWindow"
#define IMServerName		"serverName"
#define IMServerTransport	"serverTransport"
#define IMLocale		"locale"
#define IMInputStyles		"inputStyles"
#define IMProtocolHandler	"protocolHandler"
#define IMOnKeysList		"onKeysList"
#define IMOffKeysList		"offKeysList"
#define IMEncodingList		"encodingList"
#define IMFilterEventMask	"filterEventMask"
#define IMProtocolDepend	"protocolDepend"

/* Masks for IM Attributes Name */
#define I18N_IMSERVER_WIN	0x0001 /* IMServerWindow */
#define I18N_IM_NAME		0x0002 /* IMServerName */
#define I18N_IM_LOCALE		0x0004 /* IMLocale */
#define I18N_IM_ADDRESS		0x0008 /* IMServerTransport */
#define I18N_INPUT_STYLES	0x0010 /* IMInputStyles */
#define I18N_ON_KEYS		0x0020 /* IMOnKeysList */
#define I18N_OFF_KEYS		0x0040 /* IMOffKeysList */
#define I18N_IM_HANDLER		0x0080 /* IMProtocolHander */
#define I18N_ENCODINGS		0x0100 /* IMEncodingList */
#define I18N_FILTERMASK		0x0200 /* IMFilterEventMask */
#define I18N_PROTO_DEPEND	0x0400 /* IMProtoDepend */

typedef struct
{
    char	*name;
    XPointer	value;
} XIMArg;

typedef struct
{
    CARD32	keysym;
    CARD32	modifier;
    CARD32	modifier_mask;
} XIMTriggerKey;

typedef struct
{
    unsigned short count_keys;
    XIMTriggerKey *keylist;
} XIMTriggerKeys;

typedef char *XIMEncoding;

typedef struct
{
    unsigned short count_encodings;
    XIMEncoding *supported_encodings;
} XIMEncodings;

typedef struct _XIMS *XIMS;

typedef struct
{
    void*	(*setup) (Display *, XIMArg *);
    Status	(*openIM) (XIMS);
    Status	(*closeIM) (XIMS);
    char*	(*setIMValues) (XIMS, XIMArg *);
    char*	(*getIMValues) (XIMS, XIMArg *);
    Status	(*forwardEvent) (XIMS, XPointer);
    Status	(*commitString) (XIMS, XPointer);
    int		(*callCallback) (XIMS, XPointer);
    int		(*preeditStart) (XIMS, XPointer);
    int		(*preeditEnd) (XIMS, XPointer);
    int		(*syncXlib) (XIMS, XPointer);
} IMMethodsRec, *IMMethods;

typedef struct
{
    Display	*display;
    int		screen;
} IMCoreRec, *IMCore;

typedef struct _XIMS
{
    IMMethods	methods;
    IMCoreRec	core;
    Bool	sync;
    void	*protocol;
} XIMProtocolRec;

/* 
 * X function declarations.
 */
extern XIMS IMOpenIM (Display *, ...);
extern Status IMCloseIM (XIMS);
extern char *IMSetIMValues (XIMS, ...);
extern char *IMGetIMValues (XIMS, ...);
void IMForwardEvent (XIMS, XPointer);
void IMCommitString (XIMS, XPointer);
int IMCallCallback (XIMS, XPointer);
int IMPreeditStart (XIMS, XPointer);
int IMPreeditEnd (XIMS, XPointer);
int IMSyncXlib (XIMS, XPointer);

#ifdef __cplusplus
}
#endif

#endif /* IMdkit_h */
