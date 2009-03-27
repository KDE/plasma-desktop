/***************************************************************************
 *   Copyright (C) 2002~2005 by Yuking                                     *
 *   yuking_net@sohu.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "MyErrorsHandlers.h"

#include <stdio.h>
#include <signal.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_WAIT_H
#include <wait.h>
#else
#include <sys/wait.h>
#endif

#include "ime.h"
#include "tools.h"

#ifndef SIGUNUSED
#define SIGUNUSED 32
#endif

XErrorHandler   oldXErrorHandler;

//extern Bool     bLumaQQ;

void SetMyExceptionHandler (void)
{
    int             signo;

    for (signo = SIGHUP; ((signo < SIGUNUSED) && (signo!=SIGSEGV) && (signo!=SIGFPE) && (signo!=SIGINT)); signo++)
	signal (signo, OnException);
}

void OnException (int signo)
{

    fprintf (stdout, "\nFCITX -- Get Signal No.: %d\n", signo);

    switch (signo) {
    case SIGHUP:
	LoadConfig (False);
	SetIM ();
	break;
    case SIGUSR1:
    case SIGCHLD:
    case SIGWINCH:
    case SIGTTIN:
    case SIGTSTP:	
    case SIGSTOP:
    case SIGTERM:
    case SIGQUIT:
    case SIGKILL:
    default:
    	SaveIM ();
	break;
    }
}

void SetMyXErrorHandler (void)
{
    oldXErrorHandler = XSetErrorHandler (MyXErrorHandler);
}

int MyXErrorHandler (Display * dpy, XErrorEvent * event)
{
    /*这个错误信息没有任何用处，去掉好了
       char            str[256];

       XGetErrorText (dpy, event->error_code, str, 255);
       fprintf (stdout, "fcitx: %s\n", str);
     */
    if (event->error_code != 3 && event->error_code != BadMatch)	// xterm will generate 3
	oldXErrorHandler (dpy, event);

    return 0;
}
