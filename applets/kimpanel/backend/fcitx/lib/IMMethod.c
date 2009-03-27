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

/* Public Function */
void IMForwardEvent (XIMS ims, XPointer call_data)
{
    (ims->methods->forwardEvent) (ims, call_data);
}

void IMCommitString (XIMS ims, XPointer call_data)
{
    (ims->methods->commitString) (ims, call_data);
}

int IMCallCallback (XIMS ims, XPointer call_data)
{
    return (ims->methods->callCallback) (ims, call_data);
}

int IMPreeditStart (XIMS ims, XPointer call_data)
{
    return (ims->methods->preeditStart) (ims, call_data);
}

int IMPreeditEnd (XIMS ims, XPointer call_data)
{
    return (ims->methods->preeditEnd) (ims, call_data);
}

int IMSyncXlib(XIMS ims, XPointer call_data)
{
    ims->sync = True;
    return (ims->methods->syncXlib) (ims, call_data);
}
