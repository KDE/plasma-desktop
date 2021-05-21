/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "keysdata.h"

#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#include <KPluginFactory>
#include <KStandardShortcut>
#include <kglobalaccel_component_interface.h>
#include <kglobalaccel_interface.h>

KeysData::KeysData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
{
    for (int i = KStandardShortcut::AccelNone + 1; i < KStandardShortcut::StandardShortcutCount; ++i) {
        const auto id = static_cast<KStandardShortcut::StandardShortcut>(i);
        const QList<QKeySequence> activeShortcuts = KStandardShortcut::shortcut(id);
        const QList<QKeySequence> defaultShortcuts = KStandardShortcut::hardcodedDefaultShortcut(id);
        if (activeShortcuts != defaultShortcuts) {
            m_isDefault = false;
            return;
        }
    }

    KGlobalAccelInterface globalAccelInterface(QStringLiteral("org.kde.kglobalaccel"), QStringLiteral("/kglobalaccel"), QDBusConnection::sessionBus());
    if (!globalAccelInterface.isValid()) {
        return;
    }

    // Default behavior of KCModuleData is to Q_EMIT the 'aboutToLoad' after construction which
    // triggers the 'loaded' signal. Because we query the data in an async way we Q_EMIT 'loaded'
    // manually when were are done.
    disconnect(this, &KCModuleData::aboutToLoad, this, &KCModuleData::loaded);

    auto componentsWatcher = new QDBusPendingCallWatcher(globalAccelInterface.allComponents());
    connect(componentsWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QList<QDBusObjectPath>> componentsReply = *watcher;
        if (componentsReply.isError() || componentsReply.value().isEmpty()) {
            Q_EMIT loaded();
            return;
        }
        const auto components = componentsReply.value();
        for (const auto &componentPath : components) {
            KGlobalAccelComponentInterface component(QStringLiteral("org.kde.kglobalaccel"), componentPath.path(), QDBusConnection::sessionBus());
            ++m_pendingComponentCalls;
            auto shortcutsWatcher = new QDBusPendingCallWatcher(component.allShortcutInfos());
            connect(shortcutsWatcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<QList<KGlobalShortcutInfo>> shortcutsReply = *watcher;
                if (shortcutsReply.isValid()) {
                    const auto allShortcuts = shortcutsReply.value();
                    bool isNotDefault = std::any_of(allShortcuts.cbegin(), allShortcuts.cend(), [](const KGlobalShortcutInfo &info) {
                        return info.defaultKeys() != info.keys();
                    });
                    m_isDefault &= isNotDefault;
                }
                if (--m_pendingComponentCalls == 0) {
                    Q_EMIT loaded();
                }
            });
        }
    });
}

bool KeysData::isDefaults() const
{
    return m_isDefault;
}
