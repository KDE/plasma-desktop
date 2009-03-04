#include "lookuptablewidget.h"

#include <plasma/paintutils.h>
#include <kiconloader.h>
#include <QtCore>
#include <QtGui>

#ifdef Q_WS_X11
#include <QX11Info>
#include <NETRootInfo>
#include <KWindowSystem>
#include <X11/Xlib.h>
#endif

LookupTableWidget::LookupTableWidget(QWidget *parent):QWidget(parent)
{
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

    m_background_svg = new Plasma::FrameSvg(this);
//X     m_background_svg = new Plasma::Svg(this);
//X     m_factory_svg = new Plasma::Svg(this);
//X     m_config_svg = new Plasma::Svg(this);
//X     m_help_svg = new Plasma::Svg(this);

//X     m_background_svg->setImagePath("widgets/panel-background");
    m_background_svg->setImagePath("dialogs/krunner");
//X     m_factory_svg->setImagePath("widgets/button");
//X     m_config_svg->setImagePath("widgets/configuration-icons");
//X     m_help_svg->setImagePath("widgets/button");
    
    m_background_svg->setEnabledBorders(Plasma::FrameSvg::LeftBorder | Plasma::FrameSvg::RightBorder);
//X     m_background_svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background_svg, SIGNAL(repaintNeeded()), SLOT(update()));

//X     m_factory_svg->setContainsMultipleImages(true);
//X     m_factory_svg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
//X     m_config_svg->setContainsMultipleImages(true);
//X     m_config_svg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
//X     m_help_svg->setContainsMultipleImages(true);
//X     m_help_svg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
        SLOT(themeUpdated()));


    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    m_aux_label = new QLabel(this);
    m_layout->addWidget(m_aux_label);
    m_candis_label = new QLabel(this);
    m_layout->addWidget(m_candis_label);
//X     m_button_container = new QWidget(this);
//X     QHBoxLayout *m_iconbox_layout = new QHBoxLayout(m_button_container);
//X     m_iconbox_layout->setMargin(0);
//X 
//X     m_factory_button = new QToolButton(this);
//X     m_factory_button->setText(i18n("Menu"));
//X     m_factory_button->setToolTip(i18n("Choose Input Method"));
//X     m_factory_button->setIcon(m_factory_svg->pixmap());
//X     connect(m_factory_button,SIGNAL(clicked()),SIGNAL(RequestShowFactoryMenu()));
//X 
//X     m_config_button = new QToolButton(this);
//X     m_config_button->setText(i18n("Config"));
//X     m_config_button->setToolTip(i18n("Config IM"));
//X     m_config_button->setIcon(m_config_svg->pixmap("configure"));
//X     connect(m_config_button,SIGNAL(clicked()),SIGNAL(RequestShowConfig()));
//X 
//X     m_help_button = new QToolButton(this);
//X     m_help_button->setText(i18n("Help"));
//X     m_help_button->setToolTip(i18n("Help on sth."));
//X     m_help_button->setIcon(m_help_svg->pixmap());
//X     connect(m_help_button,SIGNAL(clicked()),SIGNAL(RequestShowHelp()));
//X 
//X     //connect(m_configButton, SIGNAL(clicked()), SLOT(showConfigDialog()));
//X     m_layout->addWidget( m_factory_button );
//X     m_layout->addWidget( m_button_container);
//X     m_layout->addWidget( m_config_button );
//X     m_layout->addWidget( m_help_button );

    // handle property trigger signal
//X     connect(&mapper,SIGNAL(mapped(const QString &)),SIGNAL(TriggerProperty(const QString &)));

    //setWindowFlags(Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint);
    //setWindowFlags(Qt::X11BypassWindowManagerHint);
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::StaysOnTop );
    KWindowSystem::setType( winId(), NET::Dock);

    setMouseTracking(true);

/*
    QSize size = m_background_svg->size();
    size.setHeight(size.height() - (m_background_svg->elementSize("center").height()-m_button_container->height()));
    size.setWidth(size.width() - (m_background_svg->elementSize("center").width()-m_button_container->width()));
*/
    themeUpdated();

    m_dragging = false;
}

LookupTableWidget::~LookupTableWidget()
{
}

void LookupTableWidget::updateLookupTable(const LookupTable &lookup_table)
{
    show();
    m_lookup_table = lookup_table;
    update();
}

void LookupTableWidget::updateAux(const QString &text,const QList<TextAttribute> &attrs)
{
    show();
    m_aux_text = text;
    update();
}

//X void LookupTableWidget::setEnabled(bool to_enable)
//X {
//X     foreach (QWidget *w, prop_map.values()) {
//X         m_button_container->layout()->removeWidget(w);
//X     } 
//X     QLayoutItem *child;
//X     while ((child = m_button_container->layout()->takeAt(0)) != 0) {
//X         delete child;
//X     }
//X     prop_map.clear();
//X     QSizePolicy size_policy = 
//X     setSizePolicy();
//X     m_button_container->setVisible(to_enable);
//X     
//X     if (!to_enable)
//X         resizeEvent(QResizeEvent())
//X }
//X 
//X void LookupTableWidget::updateProperty(const Property &prop)
//X {
//X     QToolButton *prop_button;
//X 
//X     if (!prop_map.contains(prop.key)) {
//X         prop_button = new QToolButton(m_button_container);
//X         //connect(prop_button,SIGNAL(clicked()),SIGNAL(RequestShowFactoryMenu()));
//X         prop_button->setStyleSheet(m_button_stylesheet);
//X         m_button_container->layout()->addWidget(prop_button);
//X         prop_map.insert(prop.key,prop_button);
//X         mapper.setMapping(prop_button,prop.key);
//X         connect(prop_button,SIGNAL(clicked()),&mapper,SLOT(map()));
//X     } else {
//X         prop_button = prop_map.value(prop.key);
//X     }
//X 
//X     QPixmap icon_pixmap;
//X     if (!prop.icon.isEmpty()) {
//X         icon_pixmap = QPixmap(prop.icon).scaled(KIconLoader::SizeSmall,KIconLoader::SizeSmall);
//X     } else {
//X         QColor textColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
//X 
//X         QRect textRect = fontMetrics().boundingRect(prop.label);
//X 
//X         QPixmap textPixmap(textRect.width(), textRect.height());
//X         textPixmap.fill(Qt::transparent);
//X         QPainter p(&textPixmap);
//X     
//X         p.setRenderHint(QPainter::TextAntialiasing,false);
//X         p.setPen(textColor);
//X         // FIXME: the center alignment here is odd: the rect should be the size needed by
//X         //        the text, but for some fonts and configurations this is off by a pixel or so
//X         //        and "centering" the text painting 'fixes' that. Need to research why
//X         //        this is the case and determine if we should be painting it differently here,
//X         //        doing soething different with the boundingRect call or if it's a problem
//X         //        in Qt itself
//X         p.drawText(textPixmap.rect(), Qt::AlignCenter, prop.label);
//X         p.end();
//X 
//X         icon_pixmap = textPixmap.scaled(KIconLoader::SizeSmall,KIconLoader::SizeSmall);
//X     }
//X 
//X     prop_button->setIcon(icon_pixmap);
//X     prop_button->setToolTip(prop.tip);
//X     //prop_button->setIcon(prop_svg->pixmap());
//X }
//X 
//X void LookupTableWidget::showProperties(bool to_show)
//X {
//X }

void LookupTableWidget::themeUpdated()
{
    kDebug()<<"Update Theme"<<Plasma::Theme::defaultTheme()->themeName();
    int left;
    int right;
    int top;
    int bottom;

    getContentsMargins(&left,&right,&top,&bottom);
    const int topHeight = qMax(0, int(m_background_svg->marginSize(Plasma::TopMargin)));
    const int leftWidth = qMax(0, int(m_background_svg->marginSize(Plasma::LeftMargin)));
    const int rightWidth = qMax(0, int(m_background_svg->marginSize(Plasma::RightMargin)));
    const int bottomHeight = qMax(0, int(m_background_svg->marginSize(Plasma::BottomMargin)));
//X     const int topHeight = qMax(0, int(m_background_svg->marginSize(Plasma::TopMargin)) - top);
//X     const int leftWidth = qMax(0, int(m_background_svg->marginSize(Plasma::LeftMargin)) - left);
//X     const int rightWidth = qMax(0, int(m_background_svg->marginSize(Plasma::RightMargin)) - right);
//X     const int bottomHeight = qMax(0, int(m_background_svg->marginSize(Plasma::BottomMargin)) - bottom);
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
//X     QColor buttonBgColor = theme->color(Plasma::Theme::BackgroundColor);
//X     m_button_stylesheet = QString("QToolButton { border: 1px solid %4; border-radius: 4px; padding: 2px;"
//X                                        " background-color: rgba(%1, %2, %3, %5); }")
//X                                       .arg(buttonBgColor.red())
//X                                       .arg(buttonBgColor.green())
//X                                       .arg(buttonBgColor.blue())
//X                                       .arg(theme->color(Plasma::Theme::HighlightColor).name(), "50%");
//X     buttonBgColor = theme->color(Plasma::Theme::TextColor);
//X     m_button_stylesheet += QString("QToolButton:hover { border: 2px solid %1; }")
//X                                .arg(theme->color(Plasma::Theme::HighlightColor).name());
//X     m_button_stylesheet += QString("QToolButton:focus { border: 2px solid %1; }")
//X                                .arg(theme->color(Plasma::Theme::HighlightColor).name());
//X     m_factory_button->setStyleSheet(m_button_stylesheet);
//X     m_config_button->setStyleSheet(m_button_stylesheet);
//X     m_help_button->setStyleSheet(m_button_stylesheet);
//X     foreach (QToolButton *btn, prop_map.values()) {
//X         btn->setStyleSheet(m_button_stylesheet);
//X     
//X     }
    //kDebug() << "stylesheet is" << m_button_stylesheet;

    QPalette p = m_candis_label->palette();
    p.setColor(QPalette::WindowText, theme->color(Plasma::Theme::TextColor));
    p.setColor(QPalette::Link, theme->color(Plasma::Theme::TextColor));
    p.setColor(QPalette::LinkVisited, theme->color(Plasma::Theme::TextColor));
    m_candis_label->setPalette(p);
    m_aux_label->setPalette(p);
//X     m_previousPage->setPalette(p);
//X     m_nextPage->setPalette(p);

    //reset the icons
//X     m_factory_button->setIcon(m_factory_svg->pixmap());
//X     m_config_button->setIcon(m_config_svg->pixmap("configure"));
//X     m_help_button->setIcon(m_help_svg->pixmap());
}

void LookupTableWidget::updateSpotLocation(int x,int y)
{
    move(x,y);
}

void LookupTableWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());
    //kDebug() << "clip rect set to: " << e->rect();

    m_background_svg->paintFrame(&p);

    m_aux_label->setText(m_aux_text);

    QString result;
    foreach (const LookupTable::Entry entry, m_lookup_table.entries) {
        result.append(QString("%1.%2 ").arg(entry.label).arg(entry.text));
    }
    m_candis_label->setText(result);
/*
    p.drawText(rect(),result);
//X     m_background_svg->paint(&p,0,0);
*/
}

void LookupTableWidget::resizeEvent(QResizeEvent *e)
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
}

void LookupTableWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (m_dragging)
        move(e->globalPos()-m_init_pos);
}

void LookupTableWidget::mousePressEvent(QMouseEvent *e)
{
    m_dragging = true;
    m_init_pos = e->pos();
    setCursor(Qt::SizeAllCursor);
}

void LookupTableWidget::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragging = false;
    setCursor(Qt::ArrowCursor);
}

bool LookupTableWidget::event(QEvent *e)
{
    if (e->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }
    
    return QWidget::event(e);
}

// vim: sw=4 sts=4 et tw=100
