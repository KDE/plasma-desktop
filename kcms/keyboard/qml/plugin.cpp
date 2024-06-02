// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

#include <flags.h>

using namespace Qt::StringLiterals;

class Plugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.plasma.private.kcm_keyboard")

public:
    using QQmlExtensionPlugin::QQmlExtensionPlugin;

    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == "org.kde.plasma.private.kcm_keyboard"_L1);
        qmlRegisterSingletonType<Flags>(uri, 1, 0, "Flags", [](QQmlEngine *, QJSEngine *) -> QObject * {
            return new Flags;
        });
    }
};

#include "plugin.moc"
