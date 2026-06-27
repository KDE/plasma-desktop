/*
    SPDX-FileCopyrightText: 2026 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QHash>
#include <QString>

/**
 * Per-environment global shortcut sets, applied optionally alongside a built-in
 * layout preset. "kwin" maps action ids in the [kwin] group of kglobalshortcutsrc
 * (verified against a live config) to a key sequence, or "none" to unbind.
 * "metaKey" selects what a lone Meta press does, written to kwinrc
 * [ModifierOnlyShortcuts] Meta.
 */
struct ShortcutPreset {
    enum MetaKey {
        Unchanged,
        Launcher, // open the application launcher (Plasma default)
        Overview, // toggle the Overview effect (GNOME/Unity "Super")
    };

    QHash<QString, QString> kwin;
    MetaKey metaKey = Unchanged;
};

inline ShortcutPreset shortcutsFor(const QString &id)
{
    if (id == QLatin1String("plasma")) {
        return {{
                    {QStringLiteral("Switch One Desktop to the Left"), QStringLiteral("Meta+Ctrl+Left")},
                    {QStringLiteral("Switch One Desktop to the Right"), QStringLiteral("Meta+Ctrl+Right")},
                    {QStringLiteral("Switch One Desktop Up"), QStringLiteral("Meta+Ctrl+Up")},
                    {QStringLiteral("Switch One Desktop Down"), QStringLiteral("Meta+Ctrl+Down")},
                    {QStringLiteral("Window One Desktop to the Left"), QStringLiteral("Meta+Ctrl+Shift+Left")},
                    {QStringLiteral("Window One Desktop to the Right"), QStringLiteral("Meta+Ctrl+Shift+Right")},
                    {QStringLiteral("Window One Desktop Up"), QStringLiteral("Meta+Ctrl+Shift+Up")},
                    {QStringLiteral("Window One Desktop Down"), QStringLiteral("Meta+Ctrl+Shift+Down")},
                    {QStringLiteral("Overview"), QStringLiteral("Meta+W")},
                    {QStringLiteral("Expose"), QStringLiteral("Ctrl+F9")},
                },
                ShortcutPreset::Launcher};
    }
    if (id == QLatin1String("macos")) {
        return {{
                    {QStringLiteral("Switch One Desktop to the Left"), QStringLiteral("Ctrl+Left")},
                    {QStringLiteral("Switch One Desktop to the Right"), QStringLiteral("Ctrl+Right")},
                    {QStringLiteral("Switch One Desktop Up"), QStringLiteral("none")},
                    {QStringLiteral("Switch One Desktop Down"), QStringLiteral("none")},
                    {QStringLiteral("Window One Desktop to the Left"), QStringLiteral("Ctrl+Shift+Left")},
                    {QStringLiteral("Window One Desktop to the Right"), QStringLiteral("Ctrl+Shift+Right")},
                    {QStringLiteral("Overview"), QStringLiteral("Ctrl+Up")}, // Mission Control
                    {QStringLiteral("Expose"), QStringLiteral("Ctrl+Down")}, // App Exposé
                },
                ShortcutPreset::Launcher};
    }
    if (id == QLatin1String("gnome")) {
        return {{
                    {QStringLiteral("Switch One Desktop Up"), QStringLiteral("Meta+PgUp")},
                    {QStringLiteral("Switch One Desktop Down"), QStringLiteral("Meta+PgDown")},
                    {QStringLiteral("Switch One Desktop to the Left"), QStringLiteral("none")},
                    {QStringLiteral("Switch One Desktop to the Right"), QStringLiteral("none")},
                    {QStringLiteral("Window One Desktop Up"), QStringLiteral("Meta+Shift+PgUp")},
                    {QStringLiteral("Window One Desktop Down"), QStringLiteral("Meta+Shift+PgDown")},
                    {QStringLiteral("Overview"), QStringLiteral("Meta+W")},
                },
                ShortcutPreset::Overview};
    }
    if (id == QLatin1String("unity")) {
        return {{
                    {QStringLiteral("Switch One Desktop to the Left"), QStringLiteral("Ctrl+Alt+Left")},
                    {QStringLiteral("Switch One Desktop to the Right"), QStringLiteral("Ctrl+Alt+Right")},
                    {QStringLiteral("Switch One Desktop Up"), QStringLiteral("Ctrl+Alt+Up")},
                    {QStringLiteral("Switch One Desktop Down"), QStringLiteral("Ctrl+Alt+Down")},
                    {QStringLiteral("Window One Desktop to the Left"), QStringLiteral("Ctrl+Alt+Shift+Left")},
                    {QStringLiteral("Window One Desktop to the Right"), QStringLiteral("Ctrl+Alt+Shift+Right")},
                    {QStringLiteral("Window One Desktop Up"), QStringLiteral("Ctrl+Alt+Shift+Up")},
                    {QStringLiteral("Window One Desktop Down"), QStringLiteral("Ctrl+Alt+Shift+Down")},
                    {QStringLiteral("Overview"), QStringLiteral("Meta+S")}, // Unity workspace switcher
                },
                ShortcutPreset::Launcher};
    }
    if (id == QLatin1String("xfce")) {
        return {{
                    {QStringLiteral("Switch One Desktop to the Left"), QStringLiteral("Ctrl+Alt+Left")},
                    {QStringLiteral("Switch One Desktop to the Right"), QStringLiteral("Ctrl+Alt+Right")},
                    {QStringLiteral("Switch One Desktop Up"), QStringLiteral("Ctrl+Alt+Up")},
                    {QStringLiteral("Switch One Desktop Down"), QStringLiteral("Ctrl+Alt+Down")},
                    {QStringLiteral("Window One Desktop to the Left"), QStringLiteral("Ctrl+Alt+Shift+Left")},
                    {QStringLiteral("Window One Desktop to the Right"), QStringLiteral("Ctrl+Alt+Shift+Right")},
                    {QStringLiteral("Window One Desktop Up"), QStringLiteral("Ctrl+Alt+Shift+Up")},
                    {QStringLiteral("Window One Desktop Down"), QStringLiteral("Ctrl+Alt+Shift+Down")},
                },
                ShortcutPreset::Launcher};
    }
    return {};
}
