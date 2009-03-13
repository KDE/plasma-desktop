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
