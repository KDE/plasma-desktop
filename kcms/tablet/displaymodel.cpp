/*
    SPDX-FileCopyrightText: 2023 Han Young <hanyoung@protonmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <ddcutil_status_codes.h>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

#include <KLocalizedString>
#include <QFile>

#include "displaymodel.h"

DisplayModel::DisplayModel(QObject *parent)
    : QAbstractListModel(parent)
{
    __uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        m_requirement = Requirements::Group;
        int ngroups = 0;
        getgrouplist(pw->pw_name, pw->pw_gid, nullptr, &ngroups);
        gid_t *groups = new gid_t[ngroups];
        getgrouplist(pw->pw_name, pw->pw_gid, groups, &ngroups);

        for (int i = 0; i < ngroups; i++) {
            struct group *gr = getgrgid(groups[i]);
            if (gr == nullptr) {
                continue;
            }
            if (memcmp(gr->gr_name, "i2c", 4) == 0) {
                m_requirement = Requirements::Success;
                break;
            }
        }
        delete[] groups;
        if (m_requirement == Requirements::Success) {
            m_requirement = Requirements::Module;
            QFile modules(QStringLiteral("/proc/modules"));
            modules.open(QIODevice::ReadOnly);
            QByteArray data = modules.readLine();
            while (!data.isEmpty()) {
                if (data.contains("i2c_dev ")) {
                    m_requirement = Requirements::Success;
                    break;
                }
                data = modules.readLine();
            }
        }
    } else {
        m_requirement = Requirements::Group;
    }

    if (m_requirement == Requirements::Success) {
        DDCA_Display_Info_List *list = nullptr;
        DDCA_Status status = ddca_get_display_info_list2(true, &list);
        if (status == 0) {
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
    }
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

bool DisplayModel::requirementsSatisfied() const
{
    return m_requirement == Requirements::Success;
}
QString DisplayModel::requirementsDescription() const
{
    switch (m_requirement) {
    case Requirements::Success:
        return QString();
    case Requirements::Group:
        return i18nc("error description in the form of inline message", "Tablet screen control disabled. Please make sure the current user is in group 'i2c'");
    case Requirements::Module:
        return i18nc("error description in the form of inline message",
                     "Tablet screen control disabled. Please make sure you have loaded the kernel module 'i2c_dev'");
    }
    Q_UNREACHABLE();
    return QString();
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
