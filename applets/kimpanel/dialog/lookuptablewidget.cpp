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
    setAttribute(Qt::WA_TranslucentBackground);
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

    m_background_svg = new Plasma::FrameSvg(this);

//X     m_background_svg->setImagePath("widgets/panel-background");
    m_background_svg->setImagePath("dialogs/background");
    
    m_background_svg->setEnabledBorders(Plasma::FrameSvg::AllBorders);
    connect(m_background_svg, SIGNAL(repaintNeeded()), SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
        SLOT(themeUpdated()));


    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    m_layout->setSizeConstraint(QLayout::SetFixedSize);

    QHBoxLayout *child_layout = new QHBoxLayout(this);
    child_layout->setMargin(0);

    m_layout->addLayout(child_layout);

    m_aux_label = new QLabel(this);
    m_aux_label->setMinimumWidth(fontMetrics().width(QString(32,' ')));
    m_aux_label->setSizePolicy(QSizePolicy::Minimum,
            QSizePolicy::Minimum);
    child_layout->addWidget(m_aux_label);

    child_layout->addSpacing(fontMetrics().width("  "));

    m_preedit_label = new QLabel(this);
    m_preedit_label->setSizePolicy(QSizePolicy::Expanding,
            QSizePolicy::Expanding);
    child_layout->addWidget(m_preedit_label);

    m_candis_label = new QLabel(this);
    m_layout->addWidget(m_candis_label);

    // handle property trigger signal
//X     connect(&mapper,SIGNAL(mapped(const QString &)),SIGNAL(TriggerProperty(const QString &)));

    //setWindowFlags(Qt::FramelessWindowHint|Qt::X11BypassWindowManagerHint);
    //setWindowFlags(Qt::X11BypassWindowManagerHint);
    KWindowSystem::setState( winId(), NET::SkipTaskbar | NET::SkipPager | NET::StaysOnTop | NET::KeepAbove);
    KWindowSystem::setType( winId(), NET::Dock);

    setMouseTracking(true);

    themeUpdated();

    m_dragging = false;

    m_preedit_caret = 0;
}

LookupTableWidget::~LookupTableWidget()
{
}

void LookupTableWidget::updateLookupTable(const LookupTable &lookup_table)
{
    m_lookup_table = lookup_table;
    if (lookup_table.entries.size() > 0) {
        show();
        update();
    }
}

void LookupTableWidget::updatePreeditCaret(int pos)
{
    kDebug() << pos;
    m_preedit_caret = pos;
}

void LookupTableWidget::updatePreeditText(const QString &text,const QList<TextAttribute> &attrs)
{
    m_preedit_text = text;
    if (!text.isEmpty()) {
        show();
        update();
    }
}

void LookupTableWidget::updateAux(const QString &text,const QList<TextAttribute> &attrs)
{
    m_aux_text = text;
    if (!text.isEmpty()) {
        show();
        update();
    }
}

void LookupTableWidget::showAux(bool to_show)
{
    m_aux_label->setVisible(to_show);
    setVisible(m_aux_label->isVisible() ||
            m_candis_label->isVisible() ||
            m_preedit_label->isVisible());
}

void LookupTableWidget::showPreedit(bool to_show)
{
    m_preedit_label->setVisible(to_show);
    setVisible(m_aux_label->isVisible() ||
            m_candis_label->isVisible() ||
            m_preedit_label->isVisible());
}

void LookupTableWidget::showLookupTable(bool to_show)
{
    m_candis_label->setVisible(to_show);
    setVisible(m_aux_label->isVisible() ||
            m_candis_label->isVisible() ||
            m_preedit_label->isVisible());
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
    m_preedit_label->setPalette(p);
//X     m_previousPage->setPalette(p);
//X     m_nextPage->setPalette(p);

    //reset the icons
//X     m_factory_button->setIcon(m_factory_svg->pixmap());
//X     m_config_button->setIcon(m_config_svg->pixmap("configure"));
//X     m_help_button->setIcon(m_help_svg->pixmap());
}

void LookupTableWidget::updateSpotLocation(int x,int y)
{
    move(qMin(x,m_desktop.screenGeometry().width()-width()),
        qMin(y,m_desktop.screenGeometry().height()-height()));
}

void LookupTableWidget::paintEvent(QPaintEvent *e)
{
    QPainter p(this);

    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source);

    p.fillRect(e->rect(),Qt::transparent);
    p.setBackgroundMode(Qt::TransparentMode);
    //kDebug() << "clip rect set to: " << e->rect();
    p.save();
    m_background_svg->paintFrame(&p);
    p.restore();

    Plasma::Theme *theme = Plasma::Theme::defaultTheme();

    m_aux_label->setText(m_aux_text);

    QString cursor_text = QString("<td bgcolor='%2'><font color='%1'>%3</font></td>")
                .arg(theme->color(Plasma::Theme::BackgroundColor).name())
                .arg(theme->color(Plasma::Theme::TextColor).name())
                .arg(' ');
    m_preedit_label->setText("<td>"+m_preedit_text.left(m_preedit_caret)+"</td>" +
            cursor_text +
            "<td>"+m_preedit_text.mid(m_preedit_caret)+"</td>");

//X     p.setColor(QPalette::WindowText, theme->color(Plasma::Theme::TextColor));
//X     p.setColor(QPalette::Link, theme->color(Plasma::Theme::TextColor));

    QString result;
    QString result_format = "<td bgcolor='%2'><font color='%1'><b>%3</b></font></td><td>%4%5</td>";
    for (int i = 0; i < m_lookup_table.entries.size(); i++) {
        LookupTable::Entry entry = m_lookup_table.entries.at(i);
        QString str = result_format
            .arg(theme->color(Plasma::Theme::BackgroundColor).name())
            .arg(theme->color(Plasma::Theme::TextColor).name())
            .arg(entry.label)
            .arg(entry.text)
            .arg((i + 1 < m_lookup_table.entries.size())?"&nbsp;&nbsp;&nbsp;&nbsp;":"");
        result.append(str);
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
    if ((x()+width()>m_desktop.availableGeometry().width()) ||
        y()+height()>m_desktop.availableGeometry().height()) {
        move(qMin(x(),m_desktop.availableGeometry().width()-width()),
            qMin(y(),m_desktop.availableGeometry().height()-height()));
    }
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
