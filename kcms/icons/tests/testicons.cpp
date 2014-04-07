/* Test program for icons setup module. */

#include <QApplication>
#include <kcomponentdata.h>
#include "icons.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    //KComponentData componentData();
    //KIconConfig *w = new KIconConfig(componentData);
    QWidget *widget = new QWidget();
    KIconConfig *w = new KIconConfig(widget);
    widget->show();
    return app.exec();
}
