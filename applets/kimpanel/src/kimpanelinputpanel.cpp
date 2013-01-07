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

#include "kimpanelinputpanel.h"
#include "kimpanelsettings.h"
#include "kimpanelinputpanelgraphics.h"

// Qt
#include <QApplication>

// KDE
#include <KWindowSystem>

KimpanelInputPanel::KimpanelInputPanel(QWidget* parent)
    : Plasma::Dialog(parent, Qt::ToolTip),
      m_widget(new KimpanelInputPanelGraphics)
{
    KWindowSystem::setState(winId(), NET::KeepAbove);
    KWindowSystem::setType(winId(), NET::Tooltip);
    setAttribute(Qt::WA_X11NetWmWindowTypeToolTip, true);
    QGraphicsScene* scene = new QGraphicsScene(this);
    scene->addItem(m_widget);

    setGraphicsWidget(m_widget);
    m_widget->show();
    connect(m_widget, SIGNAL(sizeChanged()), this, SLOT(syncToGraphicsWidget()));

    connect(m_widget, SIGNAL(selectCandidate(int)), this, SIGNAL(selectCandidate(int)));
    connect(m_widget, SIGNAL(lookupTablePageUp()), this, SIGNAL(lookupTablePageUp()));
    connect(m_widget, SIGNAL(lookupTablePageDown()), this, SIGNAL(lookupTablePageDown()));
}

KimpanelInputPanel::~KimpanelInputPanel()
{
}

void KimpanelInputPanel::showEvent(QShowEvent *e)
{
    Plasma::Dialog::showEvent(e);
    updateLocation();
}

void KimpanelInputPanel::resizeEvent(QResizeEvent* event)
{
    Plasma::Dialog::resizeEvent(event);
    updateLocation();
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
    int oy = m_spotRect.y() + m_spotRect.height();
    int y = oy + 10;
    if (y > screenRect.y() + screenRect.height()) {
        y = screenRect.height();
    }

    if (y + height() > screenRect.y() + screenRect.height()) {
        /// minus 20 to make preedit bar never overlap the input context
        y = oy - (height() + ((m_spotRect.height() == 0)?20:(m_spotRect.height() + 10)));
        m_widget->setReverse(true);
    }
    else
        m_widget->setReverse(false);

    QPoint p(x, y);
    if (p != pos())
        move(p);
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

void KimpanelInputPanel::setLookupTableLayout(int layout)
{
    m_widget->setLookupTableLayout(layout);
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

void KimpanelInputPanel::updateSizeVisibility()
{
    m_widget->updateVisible();
    bool newVisible = m_widget->markedVisible();
    if (newVisible) {
        m_widget->updateSize();
    }
    updateVisible(newVisible);
}


