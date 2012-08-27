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
    : QGraphicsView(parent),
      m_widget(new KimpanelInputPanelGraphics),
      m_backgroundSvg(new Plasma::FrameSvg(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::ToolTip);
    setAttribute(Qt::WA_X11DoNotAcceptFocus, true);
    KWindowSystem::setState(winId(), NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Tooltip);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setContentsMargins(0, 0, 0, 0);
    viewport()->setContentsMargins(0, 0, 0, 0);
    viewport()->setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);

    setScene(new QGraphicsScene);

    scene()->addItem(m_widget);
    centerOn(m_widget);

    loadTheme();

    m_backgroundSvg->setCacheAllRenderedFrames(true);

    Plasma::Theme* theme = Plasma::Theme::defaultTheme();
    connect(theme, SIGNAL(themeChanged()), this, SLOT(loadTheme()));
    connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), this, SLOT(maskBackground(bool)));
    connect(m_widget, SIGNAL(visibleChanged(bool)), this, SLOT(updateVisible(bool)));
    connect(m_widget, SIGNAL(sizeChanged()), this, SLOT(updateSize()), Qt::QueuedConnection);

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
    m_backgroundSvg->resizeFrame(size());

    if (Plasma::WindowEffects::isEffectAvailable(Plasma::WindowEffects::BlurBehind) && KimpanelSettings::self()->enableBackgroundBlur()) {
        Plasma::WindowEffects::enableBlurBehind(winId(), true, m_backgroundSvg->mask());
    } else {
        Plasma::WindowEffects::enableBlurBehind(winId(), false);
    }
    qreal left, right, top, bottom;
    m_backgroundSvg->getMargins(left, top, right, bottom);
    setSceneRect(QRectF(QPointF(left, top), size() - QSize(top + bottom, left + right)));
}

void KimpanelInputPanel::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->setClipRect(rect);
    painter->setCompositionMode(QPainter::CompositionMode_Source);
    painter->fillRect(rect, Qt::transparent);
    m_backgroundSvg->paintFrame(painter);
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
    qreal left, top, right, bottom;
    m_backgroundSvg->getMargins(left, top, right, bottom);
    QSize size = m_widget->roundSize();
    m_widget->setPos(left, top);
    QSize sizeHint = size + QSize(left + right, top + bottom);
    setSceneRect(left, right, size.width(), size.height());
    resize(sizeHint);
}

