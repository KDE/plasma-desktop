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
#include "previewsettings.h"
#include "previewbridge.h"
#include "buttonsmodel.h"

#include <KLocalizedString>

#include <QFontDatabase>

namespace KDecoration2
{

namespace Preview
{

BorderSizesModel::BorderSizesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

BorderSizesModel::~BorderSizesModel() = default;

QVariant BorderSizesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_borders.count() || index.column() != 0) {
        return QVariant();
    }
    if (role != Qt::DisplayRole && role != Qt::UserRole) {
        return QVariant();
    }
    return QVariant::fromValue<BorderSize>(m_borders.at(index.row()));
}

int BorderSizesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_borders.count();
}

QHash< int, QByteArray > BorderSizesModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
    return roles;
}

PreviewSettings::PreviewSettings(DecorationSettings *parent)
    : QObject()
    , DecorationSettingsPrivate(parent)
    , m_alphaChannelSupported(true)
    , m_onAllDesktopsAvailable(true)
    , m_closeOnDoubleClick(false)
    , m_leftButtons(new ButtonsModel(QVector<DecorationButtonType>({
            DecorationButtonType::Menu,
            DecorationButtonType::ApplicationMenu,
            DecorationButtonType::OnAllDesktops
        }), this))
    , m_rightButtons(new ButtonsModel(QVector<DecorationButtonType>({
            DecorationButtonType::ContextHelp,
            DecorationButtonType::Minimize,
            DecorationButtonType::Maximize,
            DecorationButtonType::Close
        }), this))
    , m_availableButtons(new ButtonsModel(QVector<DecorationButtonType>({
            DecorationButtonType::Menu,
            DecorationButtonType::ApplicationMenu,
            DecorationButtonType::OnAllDesktops,
            DecorationButtonType::Minimize,
            DecorationButtonType::Maximize,
            DecorationButtonType::Close,
            DecorationButtonType::ContextHelp,
            DecorationButtonType::Shade,
            DecorationButtonType::KeepBelow,
            DecorationButtonType::KeepAbove
        }), this))
    , m_borderSizes(new BorderSizesModel(this))
    , m_borderSize(int(BorderSize::Normal))
    , m_font(QFontDatabase::systemFont(QFontDatabase::TitleFont))
{
    connect(this, &PreviewSettings::alphaChannelSupportedChanged, parent, &DecorationSettings::alphaChannelSupportedChanged);
    connect(this, &PreviewSettings::onAllDesktopsAvailableChanged, parent, &DecorationSettings::onAllDesktopsAvailableChanged);
    connect(this, &PreviewSettings::closeOnDoubleClickOnMenuChanged, parent, &DecorationSettings::closeOnDoubleClickOnMenuChanged);
    connect(this, &PreviewSettings::fontChanged, parent, &DecorationSettings::fontChanged);
    auto updateLeft = [this, parent]() {
        parent->decorationButtonsLeftChanged(decorationButtonsLeft());
    };
    auto updateRight = [this, parent]() {
        parent->decorationButtonsRightChanged(decorationButtonsRight());
    };
    connect(m_leftButtons,  &QAbstractItemModel::rowsRemoved,  this, updateLeft);
    connect(m_leftButtons,  &QAbstractItemModel::rowsMoved,    this, updateLeft);
    connect(m_leftButtons,  &QAbstractItemModel::rowsInserted, this, updateLeft);
    connect(m_rightButtons, &QAbstractItemModel::rowsRemoved,  this, updateRight);
    connect(m_rightButtons, &QAbstractItemModel::rowsMoved,    this, updateRight);
    connect(m_rightButtons, &QAbstractItemModel::rowsInserted, this, updateRight);
}

PreviewSettings::~PreviewSettings() = default;

QAbstractItemModel *PreviewSettings::availableButtonsModel() const
{
    return m_availableButtons;
}

QAbstractItemModel *PreviewSettings::leftButtonsModel() const
{
    return m_leftButtons;
}

QAbstractItemModel *PreviewSettings::rightButtonsModel() const
{
    return m_rightButtons;
}

bool PreviewSettings::isAlphaChannelSupported() const
{
    return m_alphaChannelSupported;
}

bool PreviewSettings::isOnAllDesktopsAvailable() const
{
    return m_onAllDesktopsAvailable;
}

void PreviewSettings::setAlphaChannelSupported(bool supported)
{
    if (m_alphaChannelSupported == supported) {
        return;
    }
    m_alphaChannelSupported = supported;
    emit alphaChannelSupportedChanged(m_alphaChannelSupported);
}

void PreviewSettings::setOnAllDesktopsAvailable(bool available)
{
    if (m_onAllDesktopsAvailable == available) {
        return;
    }
    m_onAllDesktopsAvailable = available;
    emit onAllDesktopsAvailableChanged(m_onAllDesktopsAvailable);
}

void PreviewSettings::setCloseOnDoubleClickOnMenu(bool enabled)
{
    if (m_closeOnDoubleClick == enabled) {
        return;
    }
    m_closeOnDoubleClick = enabled;
    emit closeOnDoubleClickOnMenuChanged(enabled);
}

QVector< DecorationButtonType > PreviewSettings::decorationButtonsLeft() const
{
    return m_leftButtons->buttons();
}

QVector< DecorationButtonType > PreviewSettings::decorationButtonsRight() const
{
    return m_rightButtons->buttons();
}

void PreviewSettings::addButtonToLeft(int row)
{
    QModelIndex index = m_availableButtons->index(row);
    if (!index.isValid()) {
        return;
    }
    m_leftButtons->add(index.data(Qt::UserRole).value<DecorationButtonType>());
}

void PreviewSettings::addButtonToRight(int row)
{
    QModelIndex index = m_availableButtons->index(row);
    if (!index.isValid()) {
        return;
    }
    m_rightButtons->add(index.data(Qt::UserRole).value<DecorationButtonType>());
}

void PreviewSettings::setBorderSizesIndex(int index)
{
    if (m_borderSize == index) {
        return;
    }
    m_borderSize = index;
    emit borderSizesIndexChanged(index);
    emit decorationSettings()->borderSizeChanged(borderSize());
}

BorderSize PreviewSettings::borderSize() const
{
    return m_borderSizes->index(m_borderSize).data(Qt::UserRole).value<BorderSize>();
}

void PreviewSettings::setFont(const QFont &font)
{
    if (m_font == font) {
        return;
    }
    m_font = font;
    emit fontChanged(m_font);
}

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    connect(this, &Settings::bridgeChanged, this, &Settings::createSettings);
}

Settings::~Settings() = default;

void Settings::setBridge(PreviewBridge *bridge)
{
    if (m_bridge == bridge) {
        return;
    }
    m_bridge = bridge;
    emit bridgeChanged();
}

PreviewBridge *Settings::bridge() const
{
    return m_bridge.data();
}

void Settings::createSettings()
{
    if (m_bridge.isNull()) {
        m_settings.clear();
    } else {
        m_settings = QSharedPointer<KDecoration2::DecorationSettings>::create(m_bridge.data());
        m_previewSettings = m_bridge->lastCreatedSettings();
        m_previewSettings->setBorderSizesIndex(m_borderSize);
        connect(this, &Settings::borderSizesIndexChanged, m_previewSettings, &PreviewSettings::setBorderSizesIndex);
    }
    emit settingsChanged();
}

QSharedPointer<DecorationSettings> Settings::settings() const
{
    return m_settings;
}

DecorationSettings *Settings::settingsPointer() const
{
    return m_settings.data();
}

void Settings::setBorderSizesIndex(int index)
{
    if (m_borderSize == index) {
        return;
    }
    m_borderSize = index;
    emit borderSizesIndexChanged(m_borderSize);
}

}
}
