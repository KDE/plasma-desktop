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

#ifndef _XimFunc_h
#define _XimFunc_h

/* i18nAttr.c */
void _Xi18nInitAttrList (Xi18n i18n_core);
void _Xi18nInitExtension(Xi18n i18n_core);

/* i18nClbk.c */
int _Xi18nGeometryCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nPreeditStartCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nPreeditDrawCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nPreeditCaretCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nPreeditDoneCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nStatusStartCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nStatusDrawCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nStatusDoneCallback (XIMS ims, IMProtocol *call_data);
int _Xi18nStringConversionCallback (XIMS ims, IMProtocol *call_data);

/* i18nIc.c */
void _Xi18nChangeIC (XIMS ims, IMProtocol *call_data, unsigned char *p,
                     int create_flag);
void _Xi18nGetIC (XIMS ims, IMProtocol *call_data, unsigned char *p);

/* i18nUtil.c */
int _Xi18nNeedSwap (Xi18n i18n_core, CARD16 connect_id);
Xi18nClient *_Xi18nNewClient(Xi18n i18n_core);
Xi18nClient *_Xi18nFindClient (Xi18n i18n_core, CARD16 connect_id);
void _Xi18nDeleteClient (Xi18n i18n_core, CARD16 connect_id);
void _Xi18nSendMessage (XIMS ims, CARD16 connect_id, CARD8 major_opcode,
                        CARD8 minor_opcode, unsigned char *data, long length);
void _Xi18nSendTriggerKey (XIMS ims, CARD16 connect_id);
void _Xi18nSetEventMask (XIMS ims, CARD16 connect_id, CARD16 im_id,
                         CARD16 ic_id, CARD32 forward_mask, CARD32 sync_mask);

/* Xlib internal */
void _XRegisterFilterByType(Display*, Window, int, int,
		Bool (*filter)(Display*, Window, XEvent*, XPointer), XPointer);
void _XUnregisterFilter(Display*, Window, 
		Bool (*filter)(Display*, Window, XEvent*, XPointer), XPointer);

#endif
