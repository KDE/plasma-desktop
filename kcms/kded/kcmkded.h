/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2020 Kai Uwe Broulik <kde@broulik.de>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
