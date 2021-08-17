/*
    SPDX-FileCopyrightText: 2020 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "modulesmodel.h"

#include <QCollator>

#include <KConfig>
#include <KConfigGroup>
#include <KPluginInfo>
#include <KServiceTypeTrader>

#include <algorithm>

#include "debug.h"

ModulesModel::ModulesModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

ModulesModel::~ModulesModel() = default;

int ModulesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_data.count();
}

QVariant ModulesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return QVariant();
    }

    const auto &item = m_data.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return item.display;
    case DescriptionRole:
        return item.description;
    case TypeRole:
        return item.type;
    case AutoloadEnabledRole:
        if (item.type == KDEDConfig::AutostartType) {
            return item.autoloadEnabled;
        }
        return QVariant();
    case StatusRole: {
        if (!m_runningModulesKnown) {
            return KDEDConfig::UnknownStatus;
        }
        if (m_runningModules.contains(item.moduleName)) {
            return KDEDConfig::Running;
        }
        return KDEDConfig::NotRunning;
    }
    case ModuleNameRole:
        return item.moduleName;
    case ImmutableRole:
        return item.immutable;
    }

    return QVariant();
}

bool ModulesModel::representsDefault() const
{
    bool isDefault = true;
    for (int i = 0; i < m_data.count(); ++i) {
        auto &item = m_data[i];
        if (item.type != KDEDConfig::AutostartType || item.immutable) {
            continue;
        }
        isDefault &= item.autoloadEnabled;
    }
    return isDefault;
}

bool ModulesModel::needsSave() const
{
    bool save = false;
    for (int i = 0; i < m_data.count(); ++i) {
        auto &item = m_data[i];
        if (item.type != KDEDConfig::AutostartType || item.immutable) {
            continue;
        }
        save |= item.autoloadEnabled != item.savedAutoloadEnabled;
    }
    return save;
}

bool ModulesModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool dirty = false;

    if (!checkIndex(index)) {
        return dirty;
    }

    auto &item = m_data[index.row()];

    if (item.type != KDEDConfig::AutostartType || item.immutable) {
        return dirty;
    }

    switch (role) {
    case AutoloadEnabledRole: {
        const bool autoloadEnabled = value.toBool();
        if (item.autoloadEnabled != autoloadEnabled) {
            item.autoloadEnabled = autoloadEnabled;
            dirty = true;
        }
        Q_EMIT autoloadedModulesChanged();
        break;
    }
    }

    if (dirty) {
        Q_EMIT dataChanged(index, index, {role});
    }

    return dirty;
}

QHash<int, QByteArray> ModulesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {DescriptionRole, QByteArrayLiteral("description")},
        {TypeRole, QByteArrayLiteral("type")},
        {AutoloadEnabledRole, QByteArrayLiteral("autoloadEnabled")},
        {StatusRole, QByteArrayLiteral("status")},
        {ModuleNameRole, QByteArrayLiteral("moduleName")},
        {ImmutableRole, QByteArrayLiteral("immutable")},
    };
}

// This code was copied from kded.cpp
// TODO: move this KCM to the KDED framework and share the code?
static QVector<KPluginMetaData> availableModules()
{
    QVector<KPluginMetaData> plugins = KPluginMetaData::findPlugins(QStringLiteral("kf5/kded"));
    QSet<QString> moduleIds;
    for (const KPluginMetaData &md : qAsConst(plugins)) {
        moduleIds.insert(md.pluginId());
    }
    // also search for old .desktop based kded modules
    const KPluginInfo::List oldStylePlugins = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QStringLiteral("KDEDModule")));
    for (const KPluginInfo &info : oldStylePlugins) {
        if (moduleIds.contains(info.pluginName())) {
            qCWarning(KCM_KDED).nospace() << "kded module " << info.pluginName()
                                          << " has already been found using "
                                             "JSON metadata, please don't install the now unneeded .desktop file ("
                                          << info.entryPath() << ").";
        } else {
            qCDebug(KCM_KDED).nospace() << "kded module " << info.pluginName() << " still uses .desktop files (" << info.entryPath()
                                        << "). Please port it to JSON metadata.";
            plugins.append(info.toMetaData());
        }
    }
    return plugins;
}

// this code was copied from kded.cpp
static bool isModuleLoadedOnDemand(const KPluginMetaData &module)
{
    bool loadOnDemand = true;
    // use toVariant() since it could be string or bool in the json and QJsonObject does not convert
    QVariant p = module.rawData().value(QStringLiteral("X-KDE-Kded-load-on-demand")).toVariant();
    if (p.isValid() && p.canConvert<bool>() && (p.toBool() == false)) {
        loadOnDemand = false;
    }
    return loadOnDemand;
}

void ModulesModel::load()
{
    beginResetModel();

    m_data.clear();

    KConfig kdedrc(QStringLiteral("kded5rc"), KConfig::NoGlobals);

    QStringList knownModules;

    QVector<ModulesModelData> autostartModules;
    QVector<ModulesModelData> onDemandModules;

    const auto modules = availableModules();
    for (const KPluginMetaData &module : modules) {
        QString servicePath = module.metaDataFileName();

        // autoload defaults to false if it is not found
        const bool autoload = module.rawData().value(QStringLiteral("X-KDE-Kded-autoload")).toVariant().toBool();

        // keep estimating dbusModuleName in sync with KDEDModule (kdbusaddons) and kded (kded)
        // currently (KF5) the module name in the D-Bus object path is set by the pluginId
        const QString dbusModuleName = module.pluginId();
        qCDebug(KCM_KDED) << "reading kded info from" << servicePath << "autoload =" << autoload << "dbus module name =" << dbusModuleName;

        if (knownModules.contains(dbusModuleName)) {
            continue;
        }

        knownModules.append(dbusModuleName);

        KConfigGroup cg(&kdedrc, QStringLiteral("Module-%1").arg(dbusModuleName));
        const bool autoloadEnabled = cg.readEntry("autoload", true);
        const bool immutable = cg.isEntryImmutable("autoload");

        ModulesModelData data{module.name(), module.description(), KDEDConfig::UnknownType, autoloadEnabled, dbusModuleName, immutable, autoloadEnabled};

        // The logic has to be identical to Kded::initModules.
        // They interpret X-KDE-Kded-autoload as false if not specified
        //                X-KDE-Kded-load-on-demand as true if not specified
        if (autoload) {
            data.type = KDEDConfig::AutostartType;
            autostartModules << data;
        } else if (isModuleLoadedOnDemand(module)) {
            data.type = KDEDConfig::OnDemandType;
            onDemandModules << data;
        } else {
            qCWarning(KCM_KDED) << "kcmkded: Module " << module.name() << "from file" << module.metaDataFileName()
                                << " not loaded on demand or startup! Skipping.";
            continue;
        }
    }

    QCollator collator;
    // Otherwise "Write" daemon with quotes will be at the top
    collator.setIgnorePunctuation(true);
    auto sortAlphabetically = [&collator](const ModulesModelData &a, const ModulesModelData &b) {
        return collator.compare(a.display, b.display) < 0;
    };

    std::sort(autostartModules.begin(), autostartModules.end(), sortAlphabetically);
    std::sort(onDemandModules.begin(), onDemandModules.end(), sortAlphabetically);

    m_data << autostartModules << onDemandModules;

    endResetModel();
}

bool ModulesModel::runningModulesKnown() const
{
    return m_runningModulesKnown;
}

void ModulesModel::setRunningModulesKnown(bool known)
{
    if (m_runningModulesKnown != known) {
        m_runningModulesKnown = known;
        Q_EMIT dataChanged(index(0, 0), index(m_data.count() - 1, 0), {StatusRole});
    }
}

QStringList ModulesModel::runningModules() const
{
    return m_runningModules;
}

void ModulesModel::setRunningModules(const QStringList &runningModules)
{
    if (m_runningModules == runningModules) {
        return;
    }

    m_runningModules = runningModules;
    if (m_runningModulesKnown) {
        Q_EMIT dataChanged(index(0, 0), index(m_data.count() - 1, 0), {StatusRole});
    }
}

void ModulesModel::refreshAutoloadEnabledSavedState()
{
    for (int i = 0; i < m_data.count(); ++i) {
        auto &item = m_data[i];
        item.savedAutoloadEnabled = item.autoloadEnabled;
    }
}
