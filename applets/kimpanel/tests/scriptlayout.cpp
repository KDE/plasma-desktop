#include "kimsvgscriptlayout.h"
#include <plasma/svg.h>
#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
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
    KCmdLineOptions options;
    options.add("+svg_image");
    options.add("+layout_script");
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count() != 2) {
        printf("Usage: %s [svg image] [layout script]\n",basename(argv[0]));
        return -1;
    }

    KApplication app;
    //QString path = "dialogs/background";
    QString path = args->arg(0);
    KIM::SvgScriptLayout *layout = new KIM::SvgScriptLayout();
    layout->setImagePath(path);
    layout->setScript(args->arg(1));
    layout->doLayout(QSizeF(200,50),"center");
    QPixmap pix(layout->elementRect().size().toSize());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    layout->paint(&p,layout->elementRect());
    p.end();
    QLabel *label2 = new QLabel();
    label2->setAttribute(Qt::WA_TranslucentBackground);
    label2->setWindowFlags(Qt::FramelessWindowHint);
    label2->setPixmap(pix);
    label2->setMask(pix.mask());
    label2->show();
    return app.exec();
}
