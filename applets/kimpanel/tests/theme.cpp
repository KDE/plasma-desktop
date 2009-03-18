#include "kimtheme.h"
#include <plasma/svg.h>
#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include <QtGui>
#include <libgen.h>

class ThemeChangeTest:public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void themeUpdated()
    {
        kDebug();
    }
};
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

    KIM::Theme *theme = new KIM::Theme();
    ThemeChangeTest t;
    QObject::connect(theme,SIGNAL(themeChanged()),&t,SLOT(themeUpdated()));
    theme->setThemeName("default");
    kDebug() << "Statusbar:" << theme->statusbarImagePath() << theme->statusbarLayoutPath();
    kDebug() << "LookupTable:" << theme->lookuptableImagePath() << theme->lookuptableLayoutPath();
    kDebug() << theme->color(KIM::Theme::StatusbarTextColor).name();

    return app.exec();
}

#include "theme.moc"
