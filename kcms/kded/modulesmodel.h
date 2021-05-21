/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@pbroulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
