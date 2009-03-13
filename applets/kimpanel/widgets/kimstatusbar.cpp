#include "kimstatusbar.h"
#include "paintutils.h"

#include <kglobal.h>
#include <kapplication.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kicon.h>
#include <QtCore>
#include <QtGui>

#ifdef Q_WS_X11
#include <QX11Info>
#include <NETRootInfo>
#include <KWindowSystem>
#include <X11/Xlib.h>
#endif


KIMStatusBar::KIMStatusBar(QWidget *parent, const QList<QAction *> extra_actions):QWidget(parent)
{
    m_background = new Plasma::FrameSvg(this);

    m_background->setImagePath("widgets/panel-background");
    //m_background->setImagePath("dialogs/background");
    
    m_background->setElementPrefix("south");
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

    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene,this);

    setMouseTracking(true);

    m_scene->installEventFilter(this);
//X     QSize size = m_background->size();
//X     size.setHeight(size.height() - (m_background->elementSize("center").height()-m_button_container->height()));
//X     size.setWidth(size.width() - (m_background->elementSize("center").width()-m_button_container->width()));

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

    m_widget = 0;//new KIMStatusBarGraphics();
    //m_widget->showLogo(true);
    //m_widget->setCollapsible(true);

//X     connect(m_widget,SIGNAL(triggerProperty(const QString &)),
//X             this,SIGNAL(triggerProperty(const QString &)));

    //m_scene->addItem(m_widget);

    setContextMenuPolicy(Qt::ActionsContextMenu);

    m_extraActions = extra_actions;
    m_dragging = false;

    m_desktop = new QDesktopWidget();
    m_config = new KConfigGroup(KGlobal::config(),"KIMStatusBar");
    move(m_config->readEntry("Pos",m_desktop->availableGeometry().bottomRight()-QPoint(200,40)));

    m_timer_id = -1;

    KIconLoader::global()->newIconLoader();
    themeUpdated();

    m_resizeStartCorner = Plasma::Dialog::NoCorner;
    m_resizeCorners = Plasma::Dialog::All;
    updateResizeCorners();
}

KIMStatusBar::~KIMStatusBar()
{
}

void KIMStatusBar::setEnabled(bool to_enable)
{
    //m_button_container->setVisible(to_enable);
//X     foreach (QToolButton *w, prop_map.values()) {
//X         w->setVisible(to_enable);
//X     }
    
//X     if (!to_enable)
//X         resizeEvent(QResizeEvent())
}

void KIMStatusBar::updateProperty(const Property &prop)
{
    //m_widget->updateProperty(prop);
}

void KIMStatusBar::registerProperties(const QList<Property> &props)
{
    //m_widget->registerProperties(props);
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
#if 0
    m_button_stylesheet = QString("QToolButton { border: 1px solid %4; border-radius: 4px; padding: 2px;"
                                       " background-color: rgba(%1, %2, %3, %5); }")
                                      .arg(buttonBgColor.red())
                                      .arg(buttonBgColor.green())
                                      .arg(buttonBgColor.blue())
                                      .arg(theme->color(Plasma::Theme::HighlightColor).name(), "50%");
#endif
    m_button_stylesheet = QString("QToolButton { background-color: transparent; }");
    m_button_stylesheet += QString("QToolButton:hover { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());

#if 0
    m_button_stylesheet += QString("QToolButton:focus { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());
#endif
    foreach (QToolButton *btn, prop_map.values()) {
        btn->setStyleSheet(m_button_stylesheet);
    
    }

    m_background->setImagePath("widgets/panel-background");
    m_background->setElementPrefix("south");
    //kDebug() << "stylesheet is" << m_button_stylesheet;
    QSize widget_size;
    if (m_widget) {
        widget_size = m_widget->effectiveSizeHint(Qt::MinimumSize).toSize();
    } else {
        widget_size = QSize(0,0);
    }
    setMinimumSize(left + right + widget_size.width(),
            top + bottom + widget_size.height());

}

void KIMStatusBar::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(e->rect(),Qt::transparent);
    m_background->paintFrame(&p);
}

void KIMStatusBar::resizeEvent(QResizeEvent *e)
{
    m_background->resizeFrame(e->size());
//X     m_background->resize(e->size());
#ifdef Q_WS_X11
    /*FIXME for 4.3: now the clip mask always has to be on for disabling the KWin shadow,
    in the future something better has to be done, and enable the mask only when compositing is active
    if (!QX11Info::isCompositingManagerRunning()) {
        setMask(m_background->mask());
    }
    */
    setMask(m_background->mask());
#else
    setMask(m_background->mask());
#endif
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
    updateResizeCorners();
}


void KIMStatusBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_resizeAreas[Plasma::Dialog::NorthEast].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::NorthEast) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (m_resizeAreas[Plasma::Dialog::NorthWest].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::NorthWest) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (m_resizeAreas[Plasma::Dialog::SouthEast].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::SouthEast) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (m_resizeAreas[Plasma::Dialog::SouthWest].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::SouthWest) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (!(event->buttons() & Qt::LeftButton)) {
        m_resizeStartCorner = Plasma::Dialog::NoCorner;
        unsetCursor();
    }

    // here we take care of resize..
    if (m_resizeStartCorner != Plasma::Dialog::NoCorner) {
        int newWidth;
        int newHeight;
        QPoint position;

        switch(m_resizeStartCorner) {
            case Plasma::Dialog::NorthEast:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                position = QPoint(x(), y() + height() - newHeight);
                break;
            case Plasma::Dialog::NorthWest:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                position = QPoint(x() + width() - newWidth, y() + height() - newHeight);
                break;
            case Plasma::Dialog::SouthWest:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                position = QPoint(x() + width() - newWidth, y());
                break;
            case Plasma::Dialog::SouthEast:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                position = QPoint(x(), y());
                break;
             default:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height()));
                position = QPoint(x(), y());
                break;
        }

        setGeometry(QRect(position, QSize(newWidth, newHeight)));
    }

    QWidget::mouseMoveEvent(event);
}

void KIMStatusBar::mousePressEvent(QMouseEvent *event)
{
    if (m_resizeAreas[Plasma::Dialog::NorthEast].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::NorthEast) {
        m_resizeStartCorner = Plasma::Dialog::NorthEast;

    } else if (m_resizeAreas[Plasma::Dialog::NorthWest].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::NorthWest) {
        m_resizeStartCorner = Plasma::Dialog::NorthWest;

    } else if (m_resizeAreas[Plasma::Dialog::SouthEast].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::SouthEast) {
        m_resizeStartCorner = Plasma::Dialog::SouthEast;

    } else if (m_resizeAreas[Plasma::Dialog::SouthWest].contains(event->pos()) && m_resizeCorners & Plasma::Dialog::SouthWest) {
        m_resizeStartCorner = Plasma::Dialog::SouthWest;

    } else {
        m_resizeStartCorner = Plasma::Dialog::NoCorner;
    }

    QWidget::mousePressEvent(event);
}

void KIMStatusBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_resizeStartCorner != Plasma::Dialog::NoCorner) {

        m_resizeStartCorner = Plasma::Dialog::NoCorner;
        unsetCursor();
        //emit dialogResized();
    }

    QWidget::mouseReleaseEvent(event);
}

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
        //        icon_pixmap = KIcon(prop.icon).pixmap(KIconLoader::SizeSmall,KIconLoader::SizeSmall,Qt::KeepAspectRatio);
                icon = KIcon(prop.icon);
            } else {
                icon = KIcon(renderText(prop.label).scaled(KIconLoader::SizeSmall,KIconLoader::SizeSmall));
            }

            prop_button->setIcon(icon);
            prop_button->setToolTip(prop.tip);

            m_layout->insertWidget(m_layout->count()-m_extraActions.size(),prop_button);
            prop_map.insert(prop.key,prop_button);
            prop_mapper.setMapping(prop_button,prop.key);
            connect(prop_button,SIGNAL(clicked()),&prop_mapper,SLOT(map()));
        }
    } else {
        QWidget::timerEvent(e);
    }
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

void KIMStatusBar::updateResizeCorners()
{
    const int resizeAreaMargin = 10;
    const QRect r = rect();

//X     kDebug() << r << size() << m_view->size();
    m_resizeAreas.clear();
    if (m_resizeCorners & Plasma::Dialog::NorthEast) {
        m_resizeAreas[Plasma::Dialog::NorthEast] = QRect(r.right() - resizeAreaMargin, 0,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (m_resizeCorners & Plasma::Dialog::NorthWest) {
        m_resizeAreas[Plasma::Dialog::NorthWest] = QRect(0, 0, resizeAreaMargin, resizeAreaMargin);
    }

    if (m_resizeCorners & Plasma::Dialog::SouthEast) {
        m_resizeAreas[Plasma::Dialog::SouthEast] = QRect(r.right() - resizeAreaMargin,
                                               r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (m_resizeCorners & Plasma::Dialog::SouthWest) {
        m_resizeAreas[Plasma::Dialog::SouthWest] = QRect(0, r.bottom() - resizeAreaMargin,
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
            m_dragging = true;
            m_moved = false;
            m_init_pos = mapFromGlobal(QCursor::pos());
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
            }
            break;
        default:
            ;
        }
    }
    return QWidget::eventFilter(watched, event);
}
// vim: sw=4 sts=4 et tw=100
