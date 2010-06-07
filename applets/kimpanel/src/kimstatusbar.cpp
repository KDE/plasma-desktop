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

#include "kimstatusbar.h"
#include "kimpanelsettings.h"
#include "paintutils.h"

#include <kglobal.h>
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kicon.h>

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


KIMStatusBar::KIMStatusBar(Plasma::Corona *corona, QWidget *parent, const QList<QAction *> extra_actions)
    : QWidget(parent),
      m_desktop(new QDesktopWidget()),
      m_scene(corona)
{
    if (!m_scene) {
        m_scene = new Plasma::Corona(this);
    }

    m_background = new Plasma::FrameSvg(this);

    m_background->setImagePath("widgets/panel-background");
    //m_background->setImagePath("dialogs/background");
    
    //m_background->setEnabledBorders(Plasma::FrameSvg::NoBorder);
    m_background->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background, SIGNAL(repaintNeeded()), SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
        SLOT(themeUpdated()));

    setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);
    // can i use setState only ?
    setWindowFlags(Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint);
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::StaysOnTop );
    KWindowSystem::setType( winId(), NET::Dock);

    m_view = new QGraphicsView(m_scene,this);

    setMouseTracking(true);

    m_scene->installEventFilter(this);

    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->viewport()->setAutoFillBackground(false);
    m_view->setContentsMargins(0,0,0,0);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    m_layout->setSpacing(0);
    setLayout(m_layout);

    m_layout->addWidget(m_view);

    m_widget = 0;
    setContextMenuPolicy(Qt::ActionsContextMenu);

    m_extraActions = extra_actions;
    m_dragging = false;

    m_timer_id = -1;

    KIconLoader::global()->newIconLoader();
    themeUpdated();

    connect(KIM::Settings::self(),SIGNAL(configChanged()),this,SLOT(adjustSelf()));
    adjustSelf();

}

KIMStatusBar::~KIMStatusBar()
{
    KIM::Settings::self()->writeConfig();
}

void KIMStatusBar::themeUpdated()
{
    qreal left;
    qreal right;
    qreal top;
    qreal bottom;

    m_background->getMargins(left,top,right,bottom);
    setContentsMargins(left, top, right, bottom);

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QColor buttonBgColor = theme->color(Plasma::Theme::BackgroundColor);
//    m_background->setImagePath("widgets/panel-background");
    m_background->setElementPrefix("south");

    QSize widget_size;
    if (m_widget) {
        widget_size = m_widget->effectiveSizeHint(Qt::MinimumSize).toSize();
    } else {
        widget_size = QSize(0,0);
    }
    setMinimumSize(left + right + widget_size.width(),
            top + bottom + widget_size.height());
    setMaximumSize(left + right + widget_size.width(),
            top + bottom + widget_size.height());

}

void KIMStatusBar::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(e->rect(),Qt::transparent);
    m_background->resizeFrame(size());
    m_background->paintFrame(&p);
}

void KIMStatusBar::resizeEvent(QResizeEvent *e)
{
    m_background->resizeFrame(e->size());

    setMask(m_background->mask());
    QWidget::resizeEvent(e);
    if (m_widget) {
        m_widget->resize(m_view->size());
        m_view->setSceneRect(m_widget->mapToScene(m_widget->boundingRect()).boundingRect());
        m_view->centerOn(m_widget);
    }
    if ((width() + x() > m_desktop->availableGeometry().width()) ||
        (height() + y() > m_desktop->availableGeometry().height())) {

        move(qMin(m_desktop->availableGeometry().width()-width(),x()),
            qMin(m_desktop->availableGeometry().height()-height(),y()));
    }
}


void KIMStatusBar::mouseMoveEvent(QMouseEvent *event)
{
    QWidget::mouseMoveEvent(event);
}

void KIMStatusBar::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
}

void KIMStatusBar::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    unsetCursor();

    KIM::Settings::setFloatingStatusbarPos(pos());

    QWidget::mouseReleaseEvent(event);
}

bool KIMStatusBar::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }

    return QWidget::event(e);
}

void KIMStatusBar::setGraphicsWidget(KIMStatusBarGraphics *widget)
{    if (m_widget) {
        disconnect(m_widget,SIGNAL(iconCountChanged()),this,SLOT(adjustSelf()));
        m_scene->removeItem(m_widget);
        foreach (QAction *action,m_widget->actions()) {
            removeAction(action);
        }
        m_widget = 0;
    }
    if (widget) {
        m_widget = widget;
        foreach (QAction *action,m_widget->actions()) {
            addAction(action);
        }
        m_widget->setParent(0);
        m_scene->addItem(m_widget);
        m_scene->addOffscreenWidget(m_widget);
        connect(m_widget,SIGNAL(iconCountChanged()),this,SLOT(adjustSelf()));
        //themeUpdated();
        move(KIM::Settings::self()->floatingStatusbarPos());
        adjustSelf();
    }

}

bool KIMStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_scene) {
        QGraphicsSceneMouseEvent *e;
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            e = dynamic_cast<QGraphicsSceneMouseEvent *>(event);
            if (e->button() == Qt::LeftButton) {
                m_dragging = true;
                m_moved = false;
                m_init_pos = mapFromGlobal(QCursor::pos());
            }
            //return true;
            break;
        case QEvent::GraphicsSceneMouseRelease:
            m_dragging = false;
            unsetCursor();
            KIM::Settings::setFloatingStatusbarPos(pos());
            return m_moved;
            break;
        case QEvent::GraphicsSceneMouseMove:
            if (m_dragging) {
                setCursor(Qt::SizeAllCursor);
                move(QCursor::pos() - m_init_pos);
                m_moved = true;
            }
            break;
        default:
            ;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void KIMStatusBar::adjustSelf()
{
    if (!m_widget) {
        return;
    }
    int nIcons = m_widget->iconCount();
    qreal left, top, right, bottom;
    m_background->getMargins(left,top,right,bottom);
    qreal minHeight = KIM::Settings::preferIconSize();
    qreal minWidth = minHeight;
    QSizeF newSize;

    switch (KIM::Settings::self()->floatingStatusbarLayout()) {
    case KIM::Settings::StatusbarHorizontal:
        newSize = QSizeF(left+right+minHeight*nIcons,top+bottom+minHeight);
        setMinimumSize(newSize.toSize());
        setMaximumSize(newSize.toSize());
        break;
    case KIM::Settings::StatusbarVertical:
        newSize = QSizeF(left+right+minWidth,top+bottom+minWidth*nIcons);
        setMinimumSize(newSize.toSize());
        setMaximumSize(newSize.toSize());
        break;
    case KIM::Settings::StatusbarMatrix:
        break;
    }
    if (m_widget) {
        m_widget->resize(m_view->size());
        m_view->setSceneRect(m_widget->mapToScene(m_widget->boundingRect()).boundingRect());
        m_view->centerOn(m_widget);
    }
}


// vim: sw=4 sts=4 et tw=100

