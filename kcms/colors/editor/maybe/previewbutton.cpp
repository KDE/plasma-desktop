/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "previewbutton.h"
#include "previewbridge.h"
#include "previewclient.h"
#include "previewsettings.h"

#include <KDecoration2/Decoration>

#include <QPainter>

namespace KDecoration2
{

namespace Preview
{

PreviewButtonItem::PreviewButtonItem(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
}

PreviewButtonItem::~PreviewButtonItem() = default;

void PreviewButtonItem::setType(int type)
{
    setType(KDecoration2::DecorationButtonType(type));
}

void PreviewButtonItem::setType(KDecoration2::DecorationButtonType type)
{
    if (m_type == type) {
        return;
    }
    m_type = type;
    emit typeChanged();
}

KDecoration2::DecorationButtonType PreviewButtonItem::type() const
{
    return m_type;
}

PreviewBridge *PreviewButtonItem::bridge() const
{
    return m_bridge.data();
}

void PreviewButtonItem::setBridge(PreviewBridge *bridge)
{
    if (m_bridge == bridge) {
        return;
    }
    m_bridge = bridge;
    emit bridgeChanged();
}

Settings *PreviewButtonItem::settings() const
{
    return m_settings.data();
}

void PreviewButtonItem::setSettings(Settings *settings)
{
    if (m_settings == settings) {
        return;
    }
    m_settings = settings;
    emit settingsChanged();
}

int PreviewButtonItem::typeAsInt() const
{
    return int(m_type);
}

void PreviewButtonItem::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    createButton();
}

void PreviewButtonItem::createButton()
{
    if (m_type == KDecoration2::DecorationButtonType::Custom || m_decoration || !m_settings || !m_bridge) {
        return;
    }
    m_decoration = m_bridge->createDecoration(this);
    if (!m_decoration) {
        return;
    }
    auto client = m_bridge->lastCreatedClient();
    client->setMinimizable(true);
    client->setMaximizable(true);
    client->setActive(false);
    client->setProvidesContextHelp(true);
    m_decoration->setSettings(m_settings->settings());
    m_decoration->init();
    m_button = m_bridge->createButton(m_decoration, m_type);
    connect(this, &PreviewButtonItem::widthChanged, this, &PreviewButtonItem::syncGeometry);
    connect(this, &PreviewButtonItem::heightChanged, this, &PreviewButtonItem::syncGeometry);
    syncGeometry();
}

void PreviewButtonItem::syncGeometry()
{
    if (!m_button) {
        return;
    }
    m_button->setGeometry(QRect(0, 0, width(), height()));
}

void PreviewButtonItem::paint(QPainter *painter)
{
    if (!m_button) {
        return;
    }
    QRect size { 0, 0, (int)width(), (int)height() };
    m_button->paint(painter, size);
    painter->setCompositionMode(QPainter::CompositionMode_SourceAtop);
    painter->fillRect(size, m_color);
}

void PreviewButtonItem::setColor(const QColor &color)
{
    m_color = color;
    m_color.setAlpha(127);
    update();
}

}
}
