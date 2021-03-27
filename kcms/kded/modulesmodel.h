/*
 * Copyright (C) 2020 Kai Uwe Broulik <kde@pbroulik.de>
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

#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

#include "kcmkded.h"

struct ModulesModelData {
    QString display;
    QString description;
    KDEDConfig::ModuleType type;
    bool autoloadEnabled;
    QString moduleName;
    bool immutable;
    bool savedAutoloadEnabled;
};
Q_DECLARE_TYPEINFO(ModulesModelData, Q_MOVABLE_TYPE);

class ModulesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ModulesModel(QObject *parent);
    ~ModulesModel() override;

    enum Roles {
        DescriptionRole = Qt::UserRole + 1,
        TypeRole,
        AutoloadEnabledRole,
        StatusRole,
        ModuleNameRole,
        ImmutableRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    void load();

    bool runningModulesKnown() const;
    void setRunningModulesKnown(bool known);

    QStringList runningModules() const;
    void setRunningModules(const QStringList &runningModules);

    bool representsDefault() const;
    bool needsSave() const;
    void refreshAutoloadEnabledSavedState();

Q_SIGNALS:
    void autoloadedModulesChanged();

private:
    QVector<ModulesModelData> m_data;

    bool m_runningModulesKnown = false;
    QStringList m_runningModules;
};
