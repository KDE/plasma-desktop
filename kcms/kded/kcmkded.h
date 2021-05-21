/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2002 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KCMKDED_H
#define KCMKDED_H

#include <KQuickAddons/ConfigModule>

class QDBusServiceWatcher;

class KConfig;
class KPluginMetaData;

class ModulesModel;
class FilterProxyModel;

class OrgKdeKded5Interface;

namespace org
{
namespace kde
{
using kded5 = ::OrgKdeKded5Interface;
}
}

class KDEDConfig : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(ModulesModel *model READ model CONSTANT)
    Q_PROPERTY(FilterProxyModel *filteredModel READ filteredModel CONSTANT)

    Q_PROPERTY(bool kdedRunning READ kdedRunning NOTIFY kdedRunningChanged)

public:
    explicit KDEDConfig(QObject *parent, const QVariantList &foo = QVariantList());
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

    ModulesModel *m_model;
    FilterProxyModel *m_filteredModel;

    org::kde::kded5 *m_kdedInterface;

    QDBusServiceWatcher *m_kdedWatcher;
    bool m_kdedRunning = false;

    QString m_lastStartedModule;
    QStringList m_runningModulesBeforeReconfigure;
};

#endif // KCMKDED_H
