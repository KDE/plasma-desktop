/***************************************************************************
 *   Copyright (C) 2011 by CSSlayer <wengxt@gmail.com>                     *
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

// Qt
#include <QApplication>
#include <QVBoxLayout>
#include <QGraphicsView>
#include <QX11Info>

// KDE
#include <KWindowSystem>

// Plasma
#include <Plasma/Corona>
#include <Plasma/Theme>
#include <Plasma/WindowEffects>

#include <X11/Xlib.h>

#include "kimpanelsettings.h"
#include "kimpanelinputpanelgraphics.h"
#include "kimpanelinputpanel.h"

KimpanelInputPanel::KimpanelInputPanel(QWidget* parent)
    : QWidget(parent),
      m_layout(new QHBoxLayout(this)),
      m_scene(new QGraphicsScene(this)),
      m_view(new QGraphicsView(m_scene, this)),
      m_widget(new KimpanelInputPanelGraphics),
      m_backgroundSvg(new Plasma::FrameSvg(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::ToolTip);
    setAttribute(Qt::WA_X11DoNotAcceptFocus, true);
    KWindowSystem::setState(winId(), NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Tooltip);
    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    //m_layout->setSizeConstraint(QLayout::SetFixedSize);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setLayout(m_layout);

    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->viewport()->setAutoFillBackground(false);
    m_view->setContentsMargins(0, 0, 0, 0);
    m_view->setAttribute(Qt::WA_NoSystemBackground);
    m_view->viewport()->setAttribute(Qt::WA_NoSystemBackground);

    m_layout->addWidget(m_view);
    m_layout->addStretch();

    m_scene->addItem(m_widget);

    loadTheme();

    m_backgroundSvg->setCacheAllRenderedFrames(true);

    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    connect(theme, SIGNAL(themeChanged()), this, SLOT(loadTheme()));
    connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), this, SLOT(maskBackground(bool)));
    connect(m_widget, SIGNAL(visibleChanged(bool)), this, SLOT(updateVisible(bool)));
    connect(m_widget, SIGNAL(sizeChanged()), this, SLOT(updateSize()));

    connect(m_widget, SIGNAL(selectCandidate(int)), this, SIGNAL(selectCandidate(int)));
    connect(m_widget, SIGNAL(lookupTablePageUp()), this, SIGNAL(lookupTablePageUp()));
    connect(m_widget, SIGNAL(lookupTablePageDown()), this, SIGNAL(lookupTablePageDown()));
}

void KimpanelInputPanel::loadTheme()
{
    maskBackground(Plasma::Theme::defaultTheme()->windowTranslucencyEnabled());

    update();
}

KimpanelInputPanel::~KimpanelInputPanel()
{
}

void KimpanelInputPanel::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    Plasma::WindowEffects::overrideShadow(winId(), true);
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom( dpy, "_KDE_NET_WM_SHADOW", False );
    XDeleteProperty(dpy, winId(), atom);
}

void KimpanelInputPanel::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    QRectF rect(QPointF(0, 0), size());
    m_scene->setSceneRect(rect);
    m_backgroundSvg->resizeFrame(event->size());
    updateLocation();

    maskBackground(Plasma::Theme::defaultTheme()->windowTranslucencyEnabled());
    update();
}

void KimpanelInputPanel::maskBackground(bool composite)
{
    if (!composite) {
        m_backgroundSvg->setImagePath("opaque/dialogs/background");
        setMask(m_backgroundSvg->mask());
    } else {
        m_backgroundSvg->setImagePath("dialogs/background");
        clearMask();
    }

    if (Plasma::WindowEffects::isEffectAvailable(Plasma::WindowEffects::BlurBehind) && KimpanelSettings::self()->enableBackgroundBlur()) {
        Plasma::WindowEffects::enableBlurBehind(winId(), true, m_backgroundSvg->mask());
    } else {
        Plasma::WindowEffects::enableBlurBehind(winId(), false);
    }
    qreal left, right, top, bottom;
    m_backgroundSvg->getMargins(left, top, right, bottom);
    setContentsMargins(left, top, right, bottom);
}

void KimpanelInputPanel::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(e->rect(), Qt::transparent);
    m_backgroundSvg->paintFrame(&p);
}

void KimpanelInputPanel::setSpotLocation(const QRect& rect)
{
    m_spotRect = rect;
    updateLocation();
}

void KimpanelInputPanel::updateLocation()
{
    QRect screenRect = QApplication::desktop()->screenGeometry(QPoint(m_spotRect.x(), m_spotRect.y()));
    int x;
    x = qMin(m_spotRect.x(), screenRect.x() + screenRect.width() - width());
    x = qMax(x, screenRect.x());
    int y = m_spotRect.y() + m_spotRect.height();
    if (y > screenRect.y() + screenRect.height()) {
        y = screenRect.height();
    }

    if (y + height() > screenRect.y() + screenRect.height()) {
        /// minus 20 to make preedit bar never overlap the input context
        y -= height() + ((m_spotRect.height() == 0)?20:m_spotRect.height());
        m_widget->setReverse(true);
    }
    else
        m_widget->setReverse(false);
    if (QPoint(x, y) != pos())
        move(x, y);
}

void KimpanelInputPanel::setShowPreedit(bool show)
{
    m_widget->setShowPreedit(show);
}

void KimpanelInputPanel::setShowAux(bool show)
{
    m_widget->setShowAux(show);
}

void KimpanelInputPanel::setShowLookupTable(bool show)
{
    m_widget->setShowLookupTable(show);
}

void KimpanelInputPanel::setLookupTableCursor(int cursor)
{
    m_widget->setLookupTableCursor(cursor);
}

void KimpanelInputPanel::setPreeditCaret(int pos)
{
    m_widget->setPreeditCaret(pos);
}

void KimpanelInputPanel::setPreeditText(const QString& text,
                                        const QString& attrs)
{
    m_widget->setPreeditText(text, attrs);
}

void KimpanelInputPanel::setAuxText(const QString& text,
                                    const QString& attrs)
{
    m_widget->setAuxText(text, attrs);
}

void KimpanelInputPanel::setLookupTable(const QStringList& labels,
                                        const QStringList& candidates,
                                        bool hasPrev,
                                        bool hasNext,
                                        const QStringList& attrs
                                       )
{
    m_widget->setLookupTable(labels, candidates, hasPrev, hasNext, attrs);
}

void KimpanelInputPanel::updateVisible(bool visible)
{
    setVisible(visible);
}

void KimpanelInputPanel::updateSize()
{
    QFontMetrics fm(KimpanelSettings::self()->font());
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QSize sizeHint = m_widget->size().toSize();
    sizeHint += QSize(left + right, top + bottom);
    setMinimumSize(sizeHint);
    setMaximumSize(sizeHint);
    if (m_view->size() != m_widget->preferredSize().toSize()) {
        m_view->resize(m_widget->preferredSize().toSize());
        m_scene->setSceneRect(QRectF(QPointF(0, 0), (m_widget->preferredSize())));
        updateLocation();
        m_backgroundSvg->resizeFrame(sizeHint);
    }
}

