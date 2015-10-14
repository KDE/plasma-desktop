/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                        *
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

#include "fullscreenwindow.h"

#include <QIcon>
#include <QScreen>

#include <KWindowEffects>
#include <KWindowSystem>

FullScreenWindow::FullScreenWindow(QQuickItem *parent) : QQuickWindow(parent ? parent->window() : 0)
, m_mainItem(0)
{
    setClearBeforeRendering(true);
    setColor(QColor(0, 0, 0, 188));
    setFlags(Qt::FramelessWindowHint);

    setIcon(QIcon::fromTheme("plasma"));

    connect(&m_theme, &Plasma::Theme::themeChanged, this, &FullScreenWindow::updateTheme);

    if (parent && parent->window()) {
        connect(parent->window(), &QWindow::screenChanged, this, &FullScreenWindow::parentScreenChanged);
    }
}

FullScreenWindow::~FullScreenWindow()
{
}

QQuickItem *FullScreenWindow::mainItem() const
{
    return m_mainItem;
}

void FullScreenWindow::setMainItem(QQuickItem *mainItem)
{
    if (m_mainItem != mainItem) {
        if (m_mainItem) {
            m_mainItem->setVisible(false);
        }

        m_mainItem = mainItem;

        if (mainItem) {
            m_mainItem->setVisible(isVisible());
            m_mainItem->setParentItem(contentItem());
        }

        emit mainItemChanged();
    }
}

void FullScreenWindow::toggle() {
    if (isVisible()) {
        close();
    } else {
        resize(screen()->size());
        showFullScreen();
    }
}

bool FullScreenWindow::event(QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
    } else if (event->type() == QEvent::Show) {
        updateTheme();

        if (m_mainItem) {
            m_mainItem->setVisible(true);
        }
    } else if (event->type() == QEvent::Hide) {
        if (m_mainItem) {
            m_mainItem->setVisible(false);
        }
    } else if (event->type() == QEvent::FocusOut) {
        if (isVisible()) {
            KWindowSystem::raiseWindow(winId());
            KWindowSystem::forceActiveWindow(winId());
        }
    }

    return QQuickWindow::event(event);
}

void FullScreenWindow::updateTheme()
{
    KWindowEffects::enableBlurBehind(winId(), true);
}

void FullScreenWindow::parentScreenChanged(const QScreen *screen)
{
    if (screen) {
        setScreen(parent()->screen());
        resize(screen->size());
    }
}
