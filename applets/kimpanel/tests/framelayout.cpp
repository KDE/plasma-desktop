#include "kimsvglayout.h"
#include "kimframesvglayout.h"
#include <plasma/svg.h>
#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include <QtGui>

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
    QString path = "dialogs/background";
    KIM::FrameSvgLayout *layout = new KIM::FrameSvgLayout();
    layout->setImagePath(path);
    layout->doLayout(QSizeF(200,200));
    kDebug() << layout->elementRect();
    QPixmap pix(layout->elementRect().size().toSize());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    layout->paint(&p,layout->elementRect());
    p.end();
    QLabel *label = new QLabel();
    label->setAttribute(Qt::WA_TranslucentBackground);
    label->setWindowFlags(Qt::FramelessWindowHint);
    label->setPixmap(pix);
    label->setMask(pix.mask());
    label->show();
    return app.exec();
}
