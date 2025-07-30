/*
    SPDX-FileCopyrightText: 2023 Jeremy Whiting <jpwhiting@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicetypemodel.h"

#include <KLocalizedString>
#include <SDL2/SDL_gamecontroller.h>

#include "logging.h"

const DeviceTypeModel::DeviceType kDefaultGamepad = DeviceTypeModel::Unknown;

DeviceTypeModel::DeviceTypeModel()
{
    addType(SDL_CONTROLLER_TYPE_UNKNOWN,
            i18nc("@label Unknown controller", "Unknown Gamepad"),
            DeviceType::Unknown,
            QStringLiteral("qrc://kcm/kcm_gamecontroller/artwork/generic-gamepad.svg"));
    addType(SDL_CONTROLLER_TYPE_XBOXONE,
            i18nc("@label XBox controller", "XBox Gamepad"),
            DeviceType::Xbox,
            QStringLiteral("qrc://kcm/kcm_gamecontroller/artwork/xbox.svg"));
    addType(SDL_CONTROLLER_TYPE_PS5,
            i18nc("@label Playstation 5 controller", "DualSense"),
            DeviceType::DualSense,
            QStringLiteral("qrc://kcm/kcm_gamecontroller/artwork/ps5.svg"));

    addDefaultType(SDL_CONTROLLER_TYPE_XBOX360, DeviceType::Xbox);
    addDefaultType(SDL_CONTROLLER_TYPE_PS3, DeviceType::DualSense);
    addDefaultType(SDL_CONTROLLER_TYPE_PS4, DeviceType::DualSense);
    addDefaultType(SDL_CONTROLLER_TYPE_UNKNOWN, kDefaultGamepad);

    // TODO: Remove this and uncomment out the addType above once we have a switchpro svg/ui
    addDefaultType(SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO, kDefaultGamepad);
}

int DeviceTypeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_types.count();
}

QVariant DeviceTypeModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    // Get the SDL type from the index's row
    const int type = m_typeOrder.value(index.row());
    switch (role) {
    case CustomRoles::NameRole:
        return std::get<0>(m_types.value(type));
    case CustomRoles::TypeRole:
        return std::get<1>(m_types.value(type));
    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceTypeModel::roleNames() const
{
    return {{CustomRoles::NameRole, "name"}, {CustomRoles::TypeRole, "type"}};
}

// Get the type row we should use for the given device type
int DeviceTypeModel::rowFromType(int type)
{
    qCDebug(KCM_GAMECONTROLLER) << "Getting device type row for type: " << type;
    if (m_typeOrder.contains(type)) {
        auto row = m_typeOrder.indexOf(type);
        qCDebug(KCM_GAMECONTROLLER) << "found in typeOrder map with row: " << row;
        return row;
    } else if (m_defaultTypes.contains(type) && m_typeOrder.contains(m_defaultTypes.value(type))) {
        auto row = m_typeOrder.indexOf(m_defaultTypes.value(type));
        qCDebug(KCM_GAMECONTROLLER) << "found in defaultTypes map with row: " << row;
        return row;
    } else {
        auto row = m_typeOrder.indexOf(kDefaultGamepad);
        qCDebug(KCM_GAMECONTROLLER) << "Not found so using default: " << row;
        return row;
    }
}

DeviceTypeModel::DeviceType DeviceTypeModel::typeFromRow(const int row)
{
    if (!checkIndex(index(row, 0))) {
        return kDefaultGamepad;
    }

    const int type = m_typeOrder.value(row);
    if (m_types.contains(type)) {
        return std::get<1>(m_types[type]);
    } else {
        return kDefaultGamepad;
    }
}

QString DeviceTypeModel::pathFromRow(int row)
{
    const int type = m_typeOrder.value(row);
    QString path;
    if (m_types.contains(type)) {
        path = std::get<2>(m_types[type]);
    } else if (m_defaultTypes.contains(type) && m_typeOrder.contains(m_defaultTypes.value(type))) {
        path = std::get<2>(m_types[m_defaultTypes.value(type)]);
    } else {
        path = std::get<2>(m_types[kDefaultGamepad]);
    }
    qCDebug(KCM_GAMECONTROLLER) << "path from row: " << row << " is: " << path;

    return path;
}

void DeviceTypeModel::addType(const int type, const QString &guiName, const DeviceType &deviceType, const QString &svgPath)
{
    m_typeOrder.append(type);
    m_types.insert(type, std::tuple<QString, DeviceType, QString>(guiName, deviceType, svgPath));
}

void DeviceTypeModel::addDefaultType(const int type, const int closestMatch)
{
    m_defaultTypes.insert(type, closestMatch);
}
