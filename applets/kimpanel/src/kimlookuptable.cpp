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

#include "kimlookuptable.h"
#include "kimpanelagent.h"
#include "kimlookuptablegraphics.h"

#include <plasma/paintutils.h>
#include <kiconloader.h>

#include <QAction>
#include <QList>
#include <QPalette>
#include <QWidget>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsView>
#include <QPainter>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

#ifdef Q_WS_X11
#include <QX11Info>
#include <NETRootInfo>
#include <KWindowSystem>
#include <X11/Xlib.h>
#endif

KIMLookupTable::KIMLookupTable(PanelAgent *agent, QWidget *parent)
    :QWidget(parent),
     m_visible(false)
{
    m_panel_agent = agent;

    if (m_panel_agent) {
        connect(m_panel_agent,
            SIGNAL(updateSpotLocation(int,int)),
            this,
            SLOT(updateSpotLocation(int,int)));
    }

    setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

    m_background = new Plasma::FrameSvg(this);

//X     m_background->setImagePath("widgets/panel-background");
    m_background->setImagePath("dialogs/background");
    
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
        SLOT(themeUpdated()));

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);

    //m_layout->setSizeConstraint(QLayout::SetFixedSize);
    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    setLayout(m_layout);

    setWindowFlags(Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint);
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::StaysOnTop);
    KWindowSystem::setType( winId(), NET::Dock);

    m_scene = new QGraphicsScene(this);

    m_view = new QGraphicsView(m_scene,this);

    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->viewport()->setAutoFillBackground(false);
    m_view->setContentsMargins(0,0,0,0);

    m_layout->addWidget(m_view);

    m_widget = new KIMLookupTableGraphics(m_panel_agent);
    connect(m_widget,SIGNAL(sizeChanged()),
            this,SLOT(propagateSizeChange()));
    connect(m_widget,SIGNAL(visibleChanged(bool)),
            this,SLOT(propagateVisibleChange(bool)));

    m_scene->addItem(m_widget);
    m_scene->setSceneRect(QRectF(-10000,-10000,20000,20000));

    setMouseTracking(true);

    themeUpdated();

    m_dragging = false;

}

KIMLookupTable::~KIMLookupTable()
{
    delete m_widget;
}

void KIMLookupTable::themeUpdated()
{
    kDebug()<<"Update Theme"<<Plasma::Theme::defaultTheme()->themeName();
    qreal left;
    qreal right;
    qreal top;
    qreal bottom;

    m_background->getMargins(left,top,right,bottom);

    setContentsMargins(left, top, right, bottom);

    QSize widget_size;
    if (m_widget) {
        widget_size = m_widget->effectiveSizeHint(Qt::MinimumSize).toSize();
    } else {
        widget_size = QSize(0,0);
    }
    setMinimumSize(left + right + widget_size.width(),
            top + bottom + widget_size.height());

    kDebug() << m_view->geometry() << contentsRect() << m_background->size();
}

void KIMLookupTable::updateSpotLocation(int x,int y)
{
    move(qMin(x,m_desktop.screenGeometry().width()-width()),
        qMin(y,m_desktop.screenGeometry().height()-height()));
}

void KIMLookupTable::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(e->rect(),Qt::transparent);
    m_background->paintFrame(&p);
}

void KIMLookupTable::resizeEvent(QResizeEvent *e)
{
    m_background->resizeFrame(e->size());
    setMask(m_background->mask());

    QWidget::resizeEvent(e);

    m_view->resize(contentsRect().size());
    if (m_widget) {
        //m_widget->resize(m_view->size());
        m_view->setSceneRect(m_widget->mapToScene(m_widget->boundingRect()).boundingRect());
        m_view->centerOn(m_widget);
    }
    if ((width() + x() > m_desktop.availableGeometry().width()) ||
        (height() + y() > m_desktop.availableGeometry().height())) {

        move(qMin(m_desktop.availableGeometry().width()-width(),x()),
            qMin(m_desktop.availableGeometry().height()-height(),y()));
    }
}

bool KIMLookupTable::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }
    
    return QWidget::event(e);
}

void KIMLookupTable::propagateSizeChange()
{
    int left,top,right,bottom;
    getContentsMargins(&left,&top,&right,&bottom);
    QSize sizeHint = m_widget->preferredSize().toSize();
    sizeHint += QSize(left+right,top+bottom);
    setMinimumSize(sizeHint);
    setMaximumSize(sizeHint);
    m_view->centerOn(m_widget);
}

void KIMLookupTable::propagateVisibleChange(bool b)
{
    if (b != m_visible) {
        m_visible = b;
        setVisible(m_visible);
        m_widget->setVisible(b);
    }
}

// vim: sw=4 sts=4 et tw=100

