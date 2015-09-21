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
, m_visualParentItem(0)
, m_visualParentWindow(0)
{
    setClearBeforeRendering(true);
    setColor(QColor(0, 0, 0, 188));
    setFlags(Qt::FramelessWindowHint);

    setIcon(QIcon::fromTheme("plasma"));

    connect(&m_theme, &Plasma::Theme::themeChanged, this, &FullScreenWindow::updateTheme);
}

FullScreenWindow::~FullScreenWindow()
{
}

QQuickItem *FullScreenWindow::mainItem() const
{
    return m_mainItem;
}

void FullScreenWindow::setMainItem(QQuickItem *item)
{
    if (m_mainItem != item) {
        if (m_mainItem) {
            m_mainItem->setVisible(false);
        }

        m_mainItem = item;

        if (m_mainItem) {
            m_mainItem->setVisible(isVisible());
            m_mainItem->setParentItem(contentItem());
        }

        emit mainItemChanged();
    }
}

QQuickItem *FullScreenWindow::visualParent() const
{
    return m_visualParentItem;
}

void FullScreenWindow::setVisualParent(QQuickItem *item)
{
    if (m_visualParentItem != item) {
        if (m_visualParentItem) {
            disconnect(m_mainItem, &QQuickItem::windowChanged, this, &FullScreenWindow::visualParentWindowChanged);
        }

        m_visualParentItem = item;

        if (m_visualParentItem) {
            if (m_visualParentItem->window()) {
                visualParentWindowChanged(m_visualParentItem->window());
            }

            connect(m_mainItem, &QQuickItem::windowChanged, this, &FullScreenWindow::visualParentWindowChanged);
        }

        emit visualParentChanged();
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

void FullScreenWindow::showEvent(QShowEvent *event)
{
    updateTheme();

    if (m_mainItem) {
        m_mainItem->setVisible(true);
    }

    QQuickWindow::showEvent(event);

    KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager);
}

void FullScreenWindow::hideEvent(QHideEvent *event)
{
    if (m_mainItem) {
        m_mainItem->setVisible(false);
    }

    QQuickWindow::hideEvent(event);
}

void FullScreenWindow::updateTheme()
{
    KWindowEffects::enableBlurBehind(winId(), true);
}

void FullScreenWindow::visualParentWindowChanged(QQuickWindow *window)
{
    if (m_visualParentWindow) {
        disconnect(m_visualParentWindow, &QQuickWindow::screenChanged, this, &FullScreenWindow::visualParentScreenChanged);
    }

    m_visualParentWindow = window;

    if (m_visualParentWindow) {
        visualParentScreenChanged(m_visualParentWindow->screen());

        connect(m_visualParentWindow, &QQuickWindow::screenChanged, this, &FullScreenWindow::visualParentScreenChanged);
    }
}

void FullScreenWindow::visualParentScreenChanged(QScreen *screen)
{
    if (screen) {
        setScreen(screen);
        resize(screen->size());
    }
}
