/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "waylandtest.h"


#include <QDebug>

#include <qcommandlineparser.h>


#include <QStringList>
#include <QTimer>

#include <QX11Info>

#include <QWidget>
#include <QPushButton>
#include <QTabWidget>

#include <QQuickWidget>

static QTextStream cout(stdout);

class WaylandTestPrivate {
public:
    QStringList loglines;
    QQuickWidget* quickWidget;
};

WaylandTest::WaylandTest(QWidget* parent) :
    QDialog(parent)
{
    d = new WaylandTestPrivate;


    setupUi(this);

    updateUi();
    show();
    raise();
    log("started");

    init();
}

void WaylandTest::init()
{

    qDebug() << "init";
    d->quickWidget = new QQuickWidget(quickTab);
    d->quickWidget->setSource(QUrl("file://home/sebas/kf5/wayland/test.qml"));
    //d->quickTab->set

}

WaylandTest::~WaylandTest()
{
    delete d;
}

void WaylandTest::log(const QString& msg)
{
    qDebug() << "msg: " << msg;
    d->loglines.prepend(msg);


    logEdit->setText(d->loglines.join('\n'));
}

void WaylandTest::updateUi()
{

    if (QX11Info::isPlatformX11()) {
        infoLabel->setText("Running on X11");
    } else {
        infoLabel->setText("This might be Wayland");
    }
}


//#include "moc_statusnotifiertest.cpp"

