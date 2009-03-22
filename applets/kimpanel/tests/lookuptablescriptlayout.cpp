#include "kimsvgscriptlayout.h"
#include <plasma/svg.h>
#include <KDebug>
#include <KApplication>
#include <KCmdLineArgs>
#include <KAboutData>
#include <KIcon>
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
    QString path = args->arg(0);
    QString aux_text = QString::fromUtf8("test it");
    QString preedit_text = QString::fromUtf8("Preedit");
    QString lookuptable_text = QString::fromUtf8("1.abd 2.ff");
    QFontMetrics fm = app.fontMetrics();
    KIM::SvgScriptLayout *layout = new KIM::SvgScriptLayout();
    layout->setImagePath(path);
    layout->setScript(args->arg(1));
    layout->doLayout(fm.size(0,aux_text),"aux_area");
    layout->doLayout(fm.size(0,preedit_text),"preedit_area");
    layout->doLayout(QSizeF(16,16),"pagedown_area");
    layout->doLayout(QSizeF(16,16),"pageup_area");
    layout->doLayout(fm.size(0,lookuptable_text),"lookuptable_area");
    kDebug() << "Root rect:" << layout->elementRect();
    kDebug() << "Aux rect:" << layout->elementRect("aux_area");
    kDebug() << "Preedit rect:" << layout->elementRect("preedit_area");
    kDebug() << "PageUp rect:" << layout->elementRect("pageup_area");
    kDebug() << "PageDown rect:" << layout->elementRect("pagedown_area");
    kDebug() << "LookupTable rect:" << layout->elementRect("lookuptable_area");
    QPixmap pix(layout->elementRect().size().toSize());
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    layout->paint(&p,layout->elementRect());
    p.setPen(Qt::black);
//    p.setBrush(Qt::white);
    if (!layout->elementRect("aux_area").isNull()) {
        p.drawText(layout->elementRect("aux_area")
                .translated( -fm.boundingRect(aux_text).topLeft())
                .topLeft(),aux_text);
    } 
    if (!layout->elementRect("preedit_area").isNull()) {
        p.drawText(layout->elementRect("preedit_area")
                .translated( -fm.boundingRect(preedit_text).topLeft())
                .topLeft(),preedit_text);
    } 
    if (!layout->elementRect("pageup_area").isNull()) {
        p.drawPixmap(layout->elementRect("pageup_area").topLeft(),KIcon("arrow-left").pixmap(16,16));
    } 
    if (!layout->elementRect("pagedown_area").isNull()) {
        p.drawPixmap(layout->elementRect("pagedown_area").topLeft(),KIcon("arrow-right").pixmap(16,16));
    } 
    if (!layout->elementRect("lookuptable_area").isNull()) {
        p.drawText(layout->elementRect("lookuptable_area")
                .translated( -fm.boundingRect(lookuptable_text).topLeft())
                .topLeft(),lookuptable_text);
    } 
    p.end();
    QLabel *label2 = new QLabel();
    label2->setAttribute(Qt::WA_TranslucentBackground);
    label2->setWindowFlags(Qt::FramelessWindowHint);
    label2->setPixmap(pix);
    label2->setMask(pix.mask());
    label2->show();
    return app.exec();
}
