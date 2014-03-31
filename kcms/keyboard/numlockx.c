/****************************************************************************

 NumLockX

 $Id: main.c,v 1.10 2001/04/30 20:55:20 seli Exp $

 Copyright (C) 2000-2001 Lubos Lunak        <l.lunak@kde.org>
 Copyright (C) 2001      Oswald Buddenhagen <ossi@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#include <config-X11.h>

#include <string.h>

#include <X11/Xlib.h>

#ifdef HAVE_XTEST
#include <X11/extensions/XTest.h>
#endif

#ifdef HAVE_XKB
#include <X11/XKBlib.h>
#endif

#include <X11/keysym.h>

static
Display* dpy;

/* the XKB stuff is based on code created by Oswald Buddenhagen <ossi@kde.org> */
#ifdef HAVE_XKB
static
int xkb_init()
    {
    int xkb_opcode, xkb_event, xkb_error;
    int xkb_lmaj = XkbMajorVersion;
    int xkb_lmin = XkbMinorVersion;
    return XkbLibraryVersion( &xkb_lmaj, &xkb_lmin )
        && XkbQueryExtension( dpy, &xkb_opcode, &xkb_event, &xkb_error,
			       &xkb_lmaj, &xkb_lmin );
    }

static
unsigned int xkb_mask_modifier( XkbDescPtr xkb, const char *name )
    {
    int i;
    if( !xkb || !xkb->names )
	return 0;
    for( i = 0;
         i < XkbNumVirtualMods;
	 i++ )
	{
	char* modStr = XGetAtomName( xkb->dpy, xkb->names->vmods[i] );
	if( modStr != NULL && strcmp(name, modStr) == 0 )
	    {
	    unsigned int mask;
	    XkbVirtualModsToReal( xkb, 1 << i, &mask );
	    return mask;
	    }
	}
    return 0;
    }

static
unsigned int xkb_numlock_mask()
    {
    XkbDescPtr xkb;
    if(( xkb = XkbGetKeyboard( dpy, XkbAllComponentsMask, XkbUseCoreKbd )) != NULL )
	{
        unsigned int mask = xkb_mask_modifier( xkb, "NumLock" );
        XkbFreeKeyboard( xkb, 0, True );
        return mask;
        }
    return 0;
    }

static
int xkb_set_on()
    {
    unsigned int mask;
    if( !xkb_init())
        return 0;
    mask = xkb_numlock_mask();
    if( mask == 0 )
        return 0;
    XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, mask);
    return 1;
    }

static
int xkb_set_off()
    {
    unsigned int mask;
    if( !xkb_init())
        return 0;
    mask = xkb_numlock_mask();
    if( mask == 0 )
        return 0;
    XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, 0);
    return 1;
    }

#ifdef NUMLOCKX_STANDALONE
static
int xkb_toggle()
    {
    unsigned int mask;
    unsigned int numlockState;
    XkbStateRec xkbState;
    if( !xkb_init())
        return 0;
    mask = xkb_numlock_mask();
    if( mask == 0 )
        return 0;
    XkbGetState( dpy, XkbUseCoreKbd, &xkbState);
    numlockState = xkbState.locked_mods & mask;
    if (numlockState)
        XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, 0);
    else
        XkbLockModifiers ( dpy, XkbUseCoreKbd, mask, mask);
    return 1;
    }
#endif

#endif

#ifdef HAVE_XTEST
static
int xtest_get_numlock_state()
    {
    int i;
    int numlock_mask = 0;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    unsigned int mask;
    XModifierKeymap* map = XGetModifierMapping( dpy );
    KeyCode numlock_keycode = XKeysymToKeycode( dpy, XK_Num_Lock );
    if( numlock_keycode == NoSymbol )
        return 0;
    for( i = 0;
         i < 8;
         ++i )
        {
	if( map->modifiermap[ map->max_keypermod * i ] == numlock_keycode )
		numlock_mask = 1 << i;
	}
    XQueryPointer( dpy, DefaultRootWindow( dpy ), &dummy1, &dummy2,
        &dummy3, &dummy4, &dummy5, &dummy6, &mask );
    XFreeModifiermap( map );
    return mask & numlock_mask;
    }

static
void xtest_change_numlock()
    {
    XTestFakeKeyEvent( dpy, XKeysymToKeycode( dpy, XK_Num_Lock ), True, CurrentTime );
    XTestFakeKeyEvent( dpy, XKeysymToKeycode( dpy, XK_Num_Lock ), False, CurrentTime );
    }

static
void xtest_set_on()
    {
    if( !xtest_get_numlock_state())
        xtest_change_numlock();
    }

static
void xtest_set_off()
    {
    if( xtest_get_numlock_state())
        xtest_change_numlock();
    }

#ifdef NUMLOCKX_STANDALONE
static
void xtest_toggle()
    {
    xtest_change_numlock();
    }
#endif

#endif

static
void numlock_set_on()
    {
#ifdef HAVE_XKB
    if( xkb_set_on())
        return;
#endif
#ifdef HAVE_XTEST
    xtest_set_on();
#endif
    }

static
void numlock_set_off()
    {
#ifdef HAVE_XKB
    if( xkb_set_off())
        return;
#endif
#ifdef HAVE_XTEST
    xtest_set_off();
#endif
    }


#ifndef NUMLOCKX_STANDALONE

void numlockx_change_numlock_state(Display* dpy_, int state)
{
#ifndef HAVE_XTEST
#ifdef __GNUC__
	#warning "XTEST extension not found - numlock setting may not work reliably"
#endif
#endif

	dpy = dpy_;
    if( state ) {
    	numlock_set_on();
    }
    else {
    	numlock_set_off();
    }
}

#else

static
void numlock_toggle()
    {
#ifdef HAVE_XKB
    if( xkb_toggle())
        return;
#endif
#ifdef HAVE_XTEST
    xtest_toggle();
#endif
    }

void usage( const char* argv0 )
    {
    printf( "NumLockX " VERSION "\n"
        "(C) 2000-2001 Lubos Lunak <l.lunak@kde.org>\n"
        "(C) 2001      Oswald Buddenhagen <ossi@kde.org>\n\n"
        "Usage : %s [on|off]\n"
        "on     - turns NumLock on in X ( default )\n"
        "off    - turns NumLock off in X\n"
        "toggle - toggles the NumLock on and off in X\n"
        "\n"
        , argv0 );
    }

int main( int argc, char* argv[] )
    {
    if( argc > 2 )
        {
        usage( argv[ 0 ] );
        return 1;
        }
    dpy = XOpenDisplay( NULL );
    if( dpy == NULL )
        {
        fprintf( stderr, "Error opening display!\n" );
        return 1;
        }
    if( argc == 1 )
        numlock_set_on();
    else if( strcmp( argv[ 1 ], "on" ) == 0 )
        numlock_set_on();
    else if( strcmp( argv[ 1 ], "off" ) == 0 )
        numlock_set_off();
    else if( strcmp( argv[ 1 ], "toggle" ) == 0 )
        numlock_toggle();
    else
        {
        usage( argv[ 0 ] );
        XCloseDisplay( dpy );
        return 2;
        }
    XCloseDisplay( dpy );
    return 0;
    }

#endif
