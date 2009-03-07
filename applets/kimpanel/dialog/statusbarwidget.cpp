#include "statusbarwidget.h"
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


StatusBarWidget::StatusBarWidget(QWidget *parent):QWidget(parent)
{
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

    m_background_svg = new Plasma::FrameSvg(this);

    m_background_svg->setImagePath("widgets/panel-background");
    //m_background_svg->setImagePath("dialogs/background");
    
    m_background_svg->setElementPrefix("south");
    m_background_svg->setEnabledBorders(Plasma::FrameSvg::NoBorder);
    connect(m_background_svg, SIGNAL(repaintNeeded()), SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
        SLOT(themeUpdated()));

    m_layout = new QHBoxLayout;
    setLayout(m_layout);

    QLabel *label = new QLabel("",this);
    label->setPixmap(KIcon("start-here-kde").pixmap(KIconLoader::SizeSmall,KIconLoader::SizeSmall));
    layout()->addWidget(label);

    // handle property/helper trigger signal
    connect(&prop_mapper,SIGNAL(mapped(const QString &)),SIGNAL(triggerProperty(const QString &)));

    // can i use setState only ?
    setWindowFlags(Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint);
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::StaysOnTop );
    KWindowSystem::setType( winId(), NET::Dock);

    setMouseTracking(true);

//X     QSize size = m_background_svg->size();
//X     size.setHeight(size.height() - (m_background_svg->elementSize("center").height()-m_button_container->height()));
//X     size.setWidth(size.width() - (m_background_svg->elementSize("center").width()-m_button_container->width()));

    layout()->setSizeConstraint(QLayout::SetFixedSize);
    themeUpdated();

    KIconLoader::global()->newIconLoader();

    m_dragging = false;

    m_desktop = new QDesktopWidget();
    m_config = new KConfigGroup(KGlobal::config(),"StatusBar");
    move(m_config->readEntry("Pos",m_desktop->availableGeometry().bottomRight()-QPoint(200,40)));

    m_timer_id = -1;

    setAttribute(Qt::WA_AlwaysShowToolTips, true);

    layout()->setMargin(0);
}

StatusBarWidget::~StatusBarWidget()
{
}

void StatusBarWidget::setEnabled(bool to_enable)
{
    //m_button_container->setVisible(to_enable);
//X     foreach (QToolButton *w, prop_map.values()) {
//X         w->setVisible(to_enable);
//X     }
    
//X     if (!to_enable)
//X         resizeEvent(QResizeEvent())
}

void StatusBarWidget::updateProperty(const Property &prop)
{
    if (!prop_map.contains(prop.key)) {
        kWarning() << prop.key <<"not contained";
        return;
    }

    QToolButton *prop_button = prop_map.value(prop.key);

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
}

#if 0
void StatusBarWidget::updateAux(const QString &text,const QList<TextAttribute> &attrs)
{
}
#endif

void StatusBarWidget::registerProperties(const QList<Property> &props)
{
    m_props = props;
    m_pending_reg_properties = m_props;
    if (m_timer_id == -1) {
        m_timer_id = startTimer(0);
    }
}

void StatusBarWidget::themeUpdated()
{
//X     const int topHeight = qMax(0, int(m_background_svg->marginSize(Plasma::TopMargin)));
//X     const int leftWidth = qMax(0, int(m_background_svg->marginSize(Plasma::LeftMargin)));
//X     const int rightWidth = qMax(0, int(m_background_svg->marginSize(Plasma::RightMargin)));
//X     const int bottomHeight = qMax(0, int(m_background_svg->marginSize(Plasma::BottomMargin)));
//X     const int topHeight = qMax(0, int(m_background_svg->marginSize(Plasma::TopMargin)) - top);
//X     const int leftWidth = qMax(0, int(m_background_svg->marginSize(Plasma::LeftMargin)) - left);
//X     const int rightWidth = qMax(0, int(m_background_svg->marginSize(Plasma::RightMargin)) - right);
//X     const int bottomHeight = qMax(0, int(m_background_svg->marginSize(Plasma::BottomMargin)) - bottom);
    //setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    qreal left;
    qreal right;
    qreal top;
    qreal bottom;

    m_background_svg->getMargins(left,top,right,bottom);
    kDebug() << left;
    setContentsMargins((left>=4)?left:4, top, right, bottom);

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

    m_background_svg->setImagePath("widgets/panel-background");
    m_background_svg->setElementPrefix("south");
    //kDebug() << "stylesheet is" << m_button_stylesheet;

}

void StatusBarWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    p.fillRect(e->rect(),Qt::transparent);

    p.setClipRect(e->rect());
    //kDebug() << "clip rect set to: " << e->rect();

    m_background_svg->paintFrame(&p);
//X     m_background_svg->paint(&p,0,0);
}

void StatusBarWidget::resizeEvent(QResizeEvent *e)
{
    m_background_svg->resizeFrame(e->size());
//X     m_background_svg->resize(e->size());
#ifdef Q_WS_X11
    /*FIXME for 4.3: now the clip mask always has to be on for disabling the KWin shadow,
    in the future something better has to be done, and enable the mask only when compositing is active
    if (!QX11Info::isCompositingManagerRunning()) {
        setMask(m_background_svg->mask());
    }
    */
    setMask(m_background_svg->mask());
#else
    setMask(m_background_svg->mask());
#endif
    QWidget::resizeEvent(e);
    if ((width() + x() > m_desktop->availableGeometry().width()) ||
        (height() + y() > m_desktop->availableGeometry().height())) {

        move(qMin(m_desktop->availableGeometry().width()-width(),x()),
            qMin(m_desktop->availableGeometry().height()-height(),y()));
    }

}

void StatusBarWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging)
        move(e->globalPos()-m_init_pos);
}

void StatusBarWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragging = true;
        m_init_pos = e->pos();
        setCursor(Qt::SizeAllCursor);
    }
}

void StatusBarWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_dragging) {
        m_dragging = false;
        setCursor(Qt::ArrowCursor);
        m_config->writeEntry("Pos",e->globalPos());
        m_config->sync();   
    }
}

void StatusBarWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_timer_id) {
        killTimer(m_timer_id);
        m_timer_id = -1;

        QLayoutItem *item;
        while ( (item = layout()->takeAt(1)) != 0 ) {
    //X         kDebug() << "remove layout item";
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

            layout()->addWidget(prop_button);
            prop_map.insert(prop.key,prop_button);
            prop_mapper.setMapping(prop_button,prop.key);
            connect(prop_button,SIGNAL(clicked()),&prop_mapper,SLOT(map()));
        }
    } else {
        QWidget::timerEvent(e);
    }
}

bool StatusBarWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }
    
    return QWidget::event(e);
}

// vim: sw=4 sts=4 et tw=100
