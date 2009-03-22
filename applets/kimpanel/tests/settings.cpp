#include "kimpanelsettings.h"
#include "kimtheme.h"
#include <plasma/svg.h>
#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KConfigDialog>
#include <QtGui>
#include <libgen.h>

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
    KApplication app;

    KIM::Settings *settings = KIM::Settings::self();
    settings->setTheme("default");
    settings->writeConfig();
    return app.exec();
}

