/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "xkboptionsmodel.h"

#include <QRegularExpression>

#include <KLocalizedString>
#include <KWindowSystem>

#include "debug.h"
#include "keyboardsettings.h"
#include "x11_helper.h"
#include "xkb_rules.h"

XkbOptionsModel::XkbOptionsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

void XkbOptionsModel::setXkbOptions(const QStringList &options)
{
    if (options != m_xkbOptions) {
        beginResetModel();
        m_xkbOptions = options;
        endResetModel();
    }
}

QStringList XkbOptionsModel::xkbOptions() const
{
    return m_xkbOptions;
}

int XkbOptionsModel::columnCount(const QModelIndex &) const
{
    return 1;
}

int XkbOptionsModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return Rules::self().optionGroupInfos.count();
    }

    if (!parent.parent().isValid()) {
        return Rules::self().optionGroupInfos[parent.row()].optionInfos.count();
    }

    return 0;
}

QModelIndex XkbOptionsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return QModelIndex();
    }

    if (index.internalId() < 100) {
        return QModelIndex();
    }

    return createIndex(((index.internalId() - index.row()) / 100) - 1, index.column());
}

QModelIndex XkbOptionsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return createIndex(row, column);
    }

    return createIndex(row, column, (100 * (parent.row() + 1)) + row);
}

Qt::ItemFlags XkbOptionsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemFlags();
    }

    if (!index.parent().isValid()) {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
}

QHash<int, QByteArray> XkbOptionsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::CheckStateRole, QByteArrayLiteral("checkState")},
    };
}

QVariant XkbOptionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int row = index.row();

    if (role == Qt::DisplayRole) {
        if (!index.parent().isValid()) {
            return Rules::self().optionGroupInfos[row].description;
        } else {
            int groupRow = index.parent().row();
            const OptionGroupInfo xkbGroup = Rules::self().optionGroupInfos[groupRow];
            return xkbGroup.optionInfos[row].description;
        }
    } else if (role == Qt::CheckStateRole) {
        if (index.parent().isValid()) {
            int groupRow = index.parent().row();
            const OptionGroupInfo xkbGroup = Rules::self().optionGroupInfos[groupRow];
            const QString &xkbOptionName = xkbGroup.optionInfos[row].name;
            return m_xkbOptions.indexOf(xkbOptionName) == -1 ? Qt::Unchecked : Qt::Checked;
        } else {
            int groupRow = index.row();
            const OptionGroupInfo xkbGroup = Rules::self().optionGroupInfos[groupRow];
            for (const OptionInfo &optionInfo : xkbGroup.optionInfos) {
                if (m_xkbOptions.indexOf(optionInfo.name) != -1) {
                    return Qt::PartiallyChecked;
                }
            }
            return Qt::Unchecked;
        }
    }
    return QVariant();
}

void XkbOptionsModel::reset()
{
    beginResetModel();
    endResetModel();
}

QString XkbOptionsModel::getShortcutName(const QString &group)
{
    QRegularExpression regexp("^" + group + Rules::XKB_OPTION_GROUP_SEPARATOR);
    QStringList grpOptions = xkbOptions().filter(regexp);

    if (grpOptions.size() == 1) {
        const QString &option = grpOptions.first();
        const std::optional<OptionGroupInfo> optionGroupInfo = Rules::self().getOptionGroupInfo(group);
        const std::optional<OptionInfo> optionInfo = optionGroupInfo->getOptionInfo(option);

        if (!optionInfo || optionInfo->description == nullptr) {
            qCDebug(KCM_KEYBOARD) << "Could not find option info for " << option;
            return grpOptions.first();
        } else {
            return optionInfo->description;
        }
    }

    if (grpOptions.size() > 1) {
        return i18np("%1 shortcut", "%1 shortcuts", grpOptions.size());
    }

    return i18nc("no shortcuts defined", "None");
}

void XkbOptionsModel::clearXkbGroup(const QString &group)
{
    QStringList options = xkbOptions();
    for (int i = options.count() - 1; i >= 0; i--) {
        if (options.at(i).startsWith(group + Rules::XKB_OPTION_GROUP_SEPARATOR)) {
            options.removeAt(i);
        }
    }

    if (options != xkbOptions()) {
        setXkbOptions(options);
    }
}

void XkbOptionsModel::navigateToGroup(const QString &group)
{
    auto it = std::find_if(Rules::self().optionGroupInfos.cbegin(), Rules::self().optionGroupInfos.cend(), [group](const OptionGroupInfo &groupInfo) {
        return groupInfo.name == group;
    });

    int index = std::distance(Rules::self().optionGroupInfos.cbegin(), it);

    if (index != -1) {
        Q_EMIT navigateTo(createIndex(index, 0));
    }
}

void XkbOptionsModel::populateWithCurrentXkbOptions()
{
    if (!KWindowSystem::isPlatformX11()) {
        // TODO: implement for Wayland - query dbus maybe?
        return;
    }

    XkbConfig xkbConfig;
    QStringList xkbOptions;
    if (X11Helper::getGroupNames(QX11Info::display(), &xkbConfig, X11Helper::ALL)) {
        xkbOptions = xkbConfig.options;
    }

    setXkbOptions(xkbOptions);
}

bool XkbOptionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int groupRow = index.parent().row();
    if (groupRow < 0) {
        return false;
    }

    const OptionGroupInfo xkbGroup = Rules::self().optionGroupInfos[groupRow];
    const OptionInfo option = xkbGroup.optionInfos[index.row()];

    if (value.toInt() == Qt::Checked) {
        if (xkbGroup.exclusive) {
            // clear if exclusive (TODO: radiobutton)
            int idx = m_xkbOptions.indexOf(QRegularExpression(xkbGroup.name + ".*"));
            if (idx >= 0) {
                for (int i = 0; i < xkbGroup.optionInfos.count(); i++) {
                    if (xkbGroup.optionInfos[i].name == m_xkbOptions.at(idx)) {
                        setData(createIndex(i, index.column(), static_cast<quint32>(index.internalId()) - index.row() + i), Qt::Unchecked, role);
                        break;
                    }
                }
            }
        }
        if (m_xkbOptions.indexOf(option.name) < 0) {
            m_xkbOptions.append(option.name);
        }
    } else {
        m_xkbOptions.removeAll(option.name);
    }

    Q_EMIT dataChanged(index, index);
    Q_EMIT dataChanged(index.parent(), index.parent());
    return true;
}
#include "moc_xkboptionsmodel.cpp"
#include "xkboptionsmodel.moc"
