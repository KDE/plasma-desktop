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
#include "paintutils.h"

#include <kglobal.h>
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kicon.h>

#ifdef Q_WS_X11
#include <QX11Info>
#include <NETRootInfo>
#include <KWindowSystem>
#include <X11/Xlib.h>
#endif


KIMStatusBar::KIMStatusBar(QWidget *parent, const QList<QAction *> extra_actions):QWidget(parent)
{
    m_layout = new KIM::SvgScriptLayout();

    setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);
    // can i use setState only ?
    setWindowFlags(Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint);
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::StaysOnTop );
    KWindowSystem::setType( winId(), NET::Dock);

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene,this);

    setMouseTracking(true);

    m_scene->installEventFilter(this);

    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->viewport()->setAutoFillBackground(false);
    m_view->setContentsMargins(0,0,0,0);
    m_view->resize(200,50);

    m_widget = 0;

    setContextMenuPolicy(Qt::ActionsContextMenu);

    m_extraActions = extra_actions;
    m_dragging = false;

    m_desktop = new QDesktopWidget();
    m_config = new KConfigGroup(KGlobal::config(),"KIMStatusBar");
    move(m_config->readEntry("Pos",m_desktop->availableGeometry().bottomRight()-QPoint(200,40)));

    m_timer_id = -1;

    KIconLoader::global()->newIconLoader();

    m_resizeStartCorner = Plasma::Dialog::NoCorner;
    m_resizeCorners = Plasma::Dialog::All;
    updateResizeCorners();

    themeUpdated();
    connect(KIM::Theme::defaultTheme(),SIGNAL(themeChanged()),this,SLOT(themeUpdated()));
}

KIMStatusBar::~KIMStatusBar()
{
}

#if 0
void KIMStatusBar::setEnabled(bool to_enable)
{
}

void KIMStatusBar::updateProperty(const Property &prop)
{
    //m_widget->updateProperty(prop);
}

void KIMStatusBar::registerProperties(const QList<Property> &props)
{
    //m_widget->registerProperties(props);
}
#endif 

void KIMStatusBar::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(e->rect(),m_background,e->rect());
}

void KIMStatusBar::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    //m_layout->doLayout(e->size());
    generateBackground();
    if (!KWindowSystem::compositingActive()) {
        setMask(m_mask);
    } else {
        setMask(rect());
    }
    m_view->setGeometry(m_layout->elementRect("statusbar").toRect());
    if (m_widget) {
        m_widget->resize(m_view->size());
        m_view->setSceneRect(m_widget->mapToScene(m_widget->boundingRect()).boundingRect());
        m_view->centerOn(m_widget);
    }
    updateResizeCorners();
    update();
}


void KIMStatusBar::mouseMoveEvent(QMouseEvent *event)
{
    switch (resizeCorner(event->pos())) {
    case Plasma::Dialog::NorthEast:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case Plasma::Dialog::NorthWest:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Plasma::Dialog::SouthEast:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case Plasma::Dialog::SouthWest:
        setCursor(Qt::SizeBDiagCursor);
        break;
    default:
        if (!(event->buttons() & Qt::LeftButton)) {
            releaseMouse();
            m_resizeStartCorner = Plasma::Dialog::NoCorner;
            unsetCursor();
        } else if (m_dragging) {
            move(QCursor::pos() - m_init_pos);
            m_moved = true;
        }
    }

    // here we take care of resize..
    if (m_resizeStartCorner != Plasma::Dialog::NoCorner) {
        QRect r = m_view->geometry();
        QSizeF newSize;
        QRectF newGeometry = geometry();

        switch(m_resizeStartCorner) {
            case Plasma::Dialog::NorthEast:
                newSize = QRect(QPoint(r.left(),event->y()),
                        QPoint(event->x(),r.bottom())).size();
                m_layout->doLayout(newSize,"statusbar");
                newGeometry = m_layout->elementRect();
                r = m_layout->elementRect("statusbar").toRect();
                newGeometry.moveTopRight(event->globalPos()+
                        (m_layout->elementRect().topRight() - r.topRight()));
                break;
            case Plasma::Dialog::NorthWest:
                newSize = QRect(event->pos(),r.bottomRight()).size();
                m_layout->doLayout(newSize,"statusbar");
                newGeometry = m_layout->elementRect();
                r = m_layout->elementRect("statusbar").toRect();
                newGeometry.moveTopLeft(event->globalPos()-r.topLeft());
                break;
            case Plasma::Dialog::SouthWest:
                newSize = QRect(QPoint(event->x(),r.top()),
                        QPoint(r.right(),event->y())).size();
                m_layout->doLayout(newSize,"statusbar");
                newGeometry = m_layout->elementRect();
                r = m_layout->elementRect("statusbar").toRect();
                newGeometry.moveBottomLeft(event->globalPos()+
                        (m_layout->elementRect().bottomLeft() - r.bottomLeft()));
                break;
            case Plasma::Dialog::SouthEast:
                newSize = QRect(r.topLeft(),event->pos()).size();
                m_layout->doLayout(newSize,"statusbar");
                newGeometry = m_layout->elementRect();
                r = m_layout->elementRect("statusbar").toRect();
                newGeometry.moveBottomRight(event->globalPos()+
                        (m_layout->elementRect().bottomRight() - r.bottomRight()));
                break;
             default:
                // impossible to reach here
                break;
        }
        setGeometry(newGeometry.toRect());
        //move(position);
        //resize(m_layout->elementSize().toSize());
    }

    QWidget::mouseMoveEvent(event);
}

void KIMStatusBar::mousePressEvent(QMouseEvent *event)
{
    m_resizeStartCorner = resizeCorner(event->pos());
    if (m_resizeStartCorner == Plasma::Dialog::NoCorner) {
        m_dragging = true;
        m_moved = false;
        m_init_pos = mapFromGlobal(QCursor::pos());
        setCursor(Qt::SizeAllCursor);
    }
    QWidget::mousePressEvent(event);
}

void KIMStatusBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_resizeStartCorner != Plasma::Dialog::NoCorner) {
        releaseMouse();

        m_resizeStartCorner = Plasma::Dialog::NoCorner;
        //emit dialogResized();
    }

    m_dragging = false;
    unsetCursor();

    QWidget::mouseReleaseEvent(event);
}

#if 0
void KIMStatusBar::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_timer_id) {
        killTimer(m_timer_id);
        m_timer_id = -1;

        QLayoutItem *item;
        while ( layout()->count() > 1 + m_extraActions.size() ) {
            item = layout()->takeAt(1);
            layout()->removeItem(item);
            delete item->widget();
        }

        prop_map.clear();
        foreach (const Property &prop, m_pending_reg_properties) {
            QToolButton *prop_button = new QToolButton(this);
            prop_button->setStyleSheet(m_button_stylesheet);

            QPixmap icon_pixmap;

            KIcon icon;

            if (!prop.icon.isEmpty()) {
                icon = KIcon(prop.icon);
            } else {
                icon = KIcon(renderText(prop.label).scaled(KIconLoader::SizeSmall,KIconLoader::SizeSmall));
            }

            prop_button->setIcon(icon);
            prop_button->setToolTip(prop.tip);

            prop_map.insert(prop.key,prop_button);
            prop_mapper.setMapping(prop_button,prop.key);
            connect(prop_button,SIGNAL(clicked()),&prop_mapper,SLOT(map()));
        }
    } else {
        QWidget::timerEvent(e);
    }
}

#endif

bool KIMStatusBar::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }
    
    return QWidget::event(e);
}

void KIMStatusBar::updateResizeCorners()
{
    const int resizeAreaMargin = 10;
    const QRect r = m_layout->elementRect("statusbar").toRect();

    m_resizeAreas.clear();
    if (m_resizeCorners & Plasma::Dialog::NorthEast) {
        m_resizeAreas[Plasma::Dialog::NorthEast] = QRect(r.right() - resizeAreaMargin, r.top(),
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (m_resizeCorners & Plasma::Dialog::NorthWest) {
        m_resizeAreas[Plasma::Dialog::NorthWest] = QRect(r.left(), r.top(), resizeAreaMargin, resizeAreaMargin);
    }

    if (m_resizeCorners & Plasma::Dialog::SouthEast) {
        m_resizeAreas[Plasma::Dialog::SouthEast] = QRect(r.right() - resizeAreaMargin,
                                               r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (m_resizeCorners & Plasma::Dialog::SouthWest) {
        m_resizeAreas[Plasma::Dialog::SouthWest] = QRect(r.left(), r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }
}

void KIMStatusBar::setGraphicsWidget(KIMStatusBarGraphics *widget)
{
    if (m_widget) {
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
        m_widget->resize(m_view->size());
        //themeUpdated();
    }
}

bool KIMStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_scene) {
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            if (resizeCorner(mapFromGlobal(QCursor::pos())) != Plasma::Dialog::NoCorner) {
                grabMouse();
                return true;
            } else {
                releaseMouse();
                m_dragging = true;
                m_moved = false;
                m_init_pos = mapFromGlobal(QCursor::pos());
            }
            //return true;
            break;
        case QEvent::GraphicsSceneMouseRelease:
            m_dragging = false;
            unsetCursor();
            return m_moved;
            break;
        case QEvent::GraphicsSceneMouseMove:
            if (m_dragging) {
                setCursor(Qt::SizeAllCursor);
                move(QCursor::pos() - m_init_pos);
                m_moved = true;
            } else {
                switch (resizeCorner(mapFromGlobal(QCursor::pos()))) {
                case Plasma::Dialog::NorthEast:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                case Plasma::Dialog::NorthWest:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                case Plasma::Dialog::SouthEast:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                case Plasma::Dialog::SouthWest:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                default:
                    QGraphicsSceneMouseEvent *e = dynamic_cast<QGraphicsSceneMouseEvent *>(event);
                    if (!(e->buttons() & Qt::LeftButton)) {
                        m_resizeStartCorner = Plasma::Dialog::NoCorner;
                        unsetCursor();
                    }
                }
            }
            break;
        default:
            ;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void KIMStatusBar::generateBackground()
{
    m_background = QPixmap(m_layout->elementSize().toSize());
    m_background.fill(Qt::transparent);
    QPainter p(&m_background);
    m_layout->paint(&p);
    p.end();
    m_mask = m_background.mask();
    p.begin(&m_mask);
    p.setBrush(Qt::black);
    p.drawRect(m_layout->elementRect("statusbar"));
}

Plasma::Dialog::ResizeCorner KIMStatusBar::resizeCorner(const QPoint &p)
{
    if (m_resizeAreas[Plasma::Dialog::NorthEast].contains(p) && m_resizeCorners & Plasma::Dialog::NorthEast) {
        return Plasma::Dialog::NorthEast;
    } else if (m_resizeAreas[Plasma::Dialog::NorthWest].contains(p) && m_resizeCorners & Plasma::Dialog::NorthWest) {
        return Plasma::Dialog::NorthWest;
    } else if (m_resizeAreas[Plasma::Dialog::SouthEast].contains(p) && m_resizeCorners & Plasma::Dialog::SouthEast) {
        return Plasma::Dialog::SouthEast;
    } else if (m_resizeAreas[Plasma::Dialog::SouthWest].contains(p) && m_resizeCorners & Plasma::Dialog::SouthWest) {
        return Plasma::Dialog::SouthWest;
    } else {
        return Plasma::Dialog::NoCorner;
    }
}

void KIMStatusBar::themeUpdated()
{
    KIM::Theme *theme = KIM::Theme::defaultTheme();
    kDebug() << theme->statusbarImagePath();
    m_layout->setImagePath(theme->statusbarImagePath());
    m_layout->setScript(theme->statusbarLayoutPath());
    m_layout->themeUpdated();
    kDebug() << m_view->size();
    m_layout->doLayout(m_view->size(),"statusbar");
    generateBackground();
    resize(m_layout->elementSize().toSize());
    m_view->setGeometry(m_layout->elementRect("statusbar").toRect());
    update();
}

// vim: sw=4 sts=4 et tw=100
