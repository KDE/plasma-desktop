/*
    SPDX-FileCopyrightText: 2026 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kcm_shortcuts.h"

#include "kcm_shortcuts_debug.h"

Q_DECLARE_LOGGING_CATEGORY(KCMSHORTCUTS)
Q_LOGGING_CATEGORY(KCMKEYS, "org.kde.kcm_shortcuts", QtInfoMsg)
K_PLUGIN_FACTORY_WITH_JSON(KCMShortcutsFactory, "kcm_shortcuts.json", registerPlugin<KCMShortcuts>();)

KCMShortcuts::KCMShortcuts(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickConfigModule(parent, metaData)
{
}

#include "kcm_shortcuts.moc"
#include "moc_kcm_shortcuts.cpp"
