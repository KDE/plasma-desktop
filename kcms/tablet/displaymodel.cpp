/*
    SPDX-FileCopyrightText: 2023 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <ddcutil_status_codes.h>

#include "displaymodel.h"

DisplayModel::DisplayModel(QObject *parent)
    : QAbstractListModel(parent)
{
    DDCA_Display_Info_List *list = nullptr;
    ddca_get_display_info_list2(true, &list);
    for (int i = 0; i < list->ct; i++) {
        auto display = std::make_unique<DDCDisplay>();
        display->name = QString::fromUtf8(list->info[i].model_name);
        // if name is empty, probably can't control with ddcutil
        if (display->name.isEmpty()) {
            continue;
        }
        display->ref = list->info[i].dref;
        display->manufacturer = QString::fromUtf8(list->info[i].mfg_id);
        m_displays.push_back(std::move(display));
    }
    ddca_free_display_info_list(list);
}

int DisplayModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_displays.size();
}
QVariant DisplayModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= int(m_displays.size())) {
        return QVariant();
    }

    switch (role) {
    case Name:
        return m_displays[row]->name;
        break;
    case Manufacturer:
        return m_displays[row]->manufacturer;
        break;
    case DDCRef:
        return QVariant::fromValue(m_displays[row]->ref);
    }
    Q_UNREACHABLE();
    return {};
}
QHash<int, QByteArray> DisplayModel::roleNames() const
{
    return {{Name, "name"}, {Manufacturer, "manufacturer"}, {DDCRef, "ref"}};
}

QVariant DisplayModel::displayAt(int index)
{
    if (index < 0 || index >= (int)m_displays.size()) {
        return QVariant();
    }
    DDCA_Display_Ref_Wrapper value;
    value.ref = m_displays[index]->ref;
    return QVariant::fromValue(value);
}

ColorSpaceModel::ColorSpaceModel()
{
    auto addItem = [this](const QString &name, int value) {
        auto item = new QStandardItem(name);
        item->setData(value, Qt::UserRole);
        appendRow(item);
    };

    const std::array<std::pair<QString, int>, 8> data = {{{QStringLiteral("Adobe RGC"), 13},
                                                          {QStringLiteral("sRGB"), 6},
                                                          {QStringLiteral("Rec 709"), 7},
                                                          {QStringLiteral("DCI P3"), 8},
                                                          {QStringLiteral("Rec 2020"), 9},
                                                          {QStringLiteral("EBU"), 10},
                                                          {QStringLiteral("Wacom Color Manager 1"), 11},
                                                          {QStringLiteral("Wacom Color Manager 2"), 12}}};
    for (const auto &[name, value] : data) {
        addItem(name, value);
    }
}

int ColorSpaceModel::indexFromColorspaceValue(int value) const
{
    const auto count = rowCount();
    for (int i = 0; i < count; i++) {
        const auto m_item = item(i);
        int itemVal = m_item->data(Qt::UserRole).toInt();
        if (itemVal == value) {
            return i;
        }
    }
    return -1;
}

QHash<int, QByteArray> ColorSpaceModel::roleNames() const
{
    return {{Qt::DisplayRole, "display"}, {Qt::UserRole, "rawValue"}};
}
