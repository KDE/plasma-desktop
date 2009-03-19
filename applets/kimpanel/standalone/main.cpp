/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include "paneldialog.h"

#ifdef Q_WS_X11
#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;
#endif

void checkComposite()
{
#ifdef Q_WS_X11
    // thanks to zack rusin and frederik for pointing me in the right direction
    // for the following bits of X11 code
    dpy = XOpenDisplay(0); // open default display
    if (!dpy)
    {
        kError() << "Cannot connect to the X server";
        return;
    }
    if( qgetenv( "KDE_SKIP_ARGB_VISUALS" ) == "1" )
        return;

    int screen = DefaultScreen(dpy);
    int eventBase, errorBase;

    if (XRenderQueryExtension(dpy, &eventBase, &errorBase))
    {
        int nvi;
        XVisualInfo templ;
        templ.screen  = screen;
        templ.depth   = 32;
        templ.c_class = TrueColor;
        XVisualInfo *xvi = XGetVisualInfo(dpy, VisualScreenMask |
                                                VisualDepthMask |
                                                VisualClassMask,
                                            &templ, &nvi);
        for (int i = 0; i < nvi; ++i)
        {
            XRenderPictFormat *format = XRenderFindVisualFormat(dpy,
                                                                xvi[i].visual);
            if (format->type == PictTypeDirect && format->direct.alphaMask)
            {
                visual = xvi[i].visual;
                colormap = XCreateColormap(dpy, RootWindow(dpy, screen),
                                            visual, AllocNone);
                break;
            }
        }

    }
#endif
}

int main(int argc, char *argv[])
{
    KAboutData about_data("kimpanel","",
                          ki18n("Input Method Panel"),
                          "0.1.0",
                          ki18n("Generic input method panel"),
                          KAboutData::License_LGPL_V3,
                          ki18n("Copyright (C) 2009, Wang Hoi")
                          );
    KCmdLineArgs::init(argc,argv,&about_data);
    KComponentData kcomp(&about_data);

    checkComposite();
#ifdef Q_WS_X11
    KApplication app(dpy, visual ? Qt::HANDLE(visual) : 0, colormap ? Qt::HANDLE(colormap) : 0);
#else
    KApplication app(0,0,0);
#endif

    KIMPanel panel;

    return app.exec();
}
