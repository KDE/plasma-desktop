/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2002 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <KQuickConfigModule>

class QDBusServiceWatcher;

class KConfig;
class KPluginMetaData;

class ModulesModel;
class FilterProxyModel;

class OrgKdeKded6Interface;

namespace org
{
namespace kde
{
using kded6 = ::OrgKdeKded6Interface;
}
}

class KDEDConfig : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(ModulesModel *model READ model CONSTANT)
    Q_PROPERTY(FilterProxyModel *filteredModel READ filteredModel CONSTANT)

    Q_PROPERTY(bool kdedRunning READ kdedRunning NOTIFY kdedRunningChanged)

public:
    explicit KDEDConfig(QObject *parent, const KPluginMetaData &metaData);
    ~KDEDConfig() override
    {
    }

    enum ModuleType {
        UnknownType = -1,
        AutostartType,
        OnDemandType,
    };
    Q_ENUM(ModuleType)

    enum ModuleStatus {
        UnknownStatus = -1,
        NotRunning,
        Running,
    };
    Q_ENUM(ModuleStatus)

    ModulesModel *model() const;
    FilterProxyModel *filteredModel() const;

    bool kdedRunning() const;

    Q_INVOKABLE void startModule(const QString &moduleName);
    Q_INVOKABLE void stopModule(const QString &moduleName);

    void load() override;
    void save() override;
    void defaults() override;

Q_SIGNALS:
    void kdedRunningChanged();

    void errorMessage(const QString &errorString);
    void showSelfDisablingModulesHint();
    void showRunningModulesChangedAfterSaveHint();

private:
    void setKdedRunning(bool kdedRunning);

    void getModuleStatus();
    void startOrStopModule(const QString &moduleName, ModuleStatus status /*better than a bool*/);

    ModulesModel *const m_model;
    FilterProxyModel *const m_filteredModel;

    org::kde::kded6 *const m_kdedInterface;

    QDBusServiceWatcher *const m_kdedWatcher;
    bool m_kdedRunning = false;

    QString m_lastStartedModule;
    QStringList m_runningModulesBeforeReconfigure;
};
