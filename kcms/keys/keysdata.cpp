/*
 * Copyright 2020 David Redondo <kde@david-redondo.de>
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

#include "keysdata.h"

#include <KGlobalAccel>
#include <KGlobalShortcutInfo>
#include <KPluginFactory>
#include <KStandardShortcut>
#include <kglobalaccel_component_interface.h>
#include <kglobalaccel_interface.h>

// Short timeout, rather fail than block isDefaults for to long which needs to be sync
constexpr int dbusTimeout = 5; // milliseconds

KeysData::KeysData(QObject *parent, const QVariantList &args)
{
    Q_UNUSED(parent)
    Q_UNUSED(args)
}

bool KeysData::isDefaults() const
{
    for (int i = KStandardShortcut::AccelNone + 1; i < KStandardShortcut::StandardShortcutCount; ++i) {
        const auto id = static_cast<KStandardShortcut::StandardShortcut>(i);
        const QList<QKeySequence> activeShortcuts = KStandardShortcut::shortcut(id);
        const QList<QKeySequence> defaultShortcuts = KStandardShortcut::hardcodedDefaultShortcut(id);
        if (activeShortcuts != defaultShortcuts) {
            return false;
        }
    }

    // need to do this blocking
    KGlobalAccelInterface globalAccelInterface(QStringLiteral("org.kde.kglobalaccel"), QStringLiteral("/kglobalaccel"), QDBusConnection::sessionBus());
    globalAccelInterface.setTimeout(dbusTimeout);
    if (!globalAccelInterface.isValid()) {
        return true;
    }
    auto componentsCall = globalAccelInterface.allComponents();
    componentsCall.waitForFinished();
    if (componentsCall.isError()) {
        return true;
    }
    const auto components = componentsCall.value();
    for (const auto &componentPath : components) {
        KGlobalAccelComponentInterface component(globalAccelInterface.service(), componentPath.path(), QDBusConnection::sessionBus());
        component.setTimeout(dbusTimeout);
        if (!component.isValid()) {
            return true;
        }
        auto allShortcutsCall = component.allShortcutInfos();
        allShortcutsCall.waitForFinished();
        if (allShortcutsCall.isError()) {
            return true;
        }
        const auto allShortcuts = allShortcutsCall.value();
        for (const auto &shortcutInfo : allShortcuts) {
            if (shortcutInfo.defaultKeys() != shortcutInfo.keys()) {
                return false;
            }
        }
    }
    return true;
}

#include "keysdata.moc"
