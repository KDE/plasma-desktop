/*
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2020 Méven Car <meven.car@kdemail.net>
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserterminal.h"

#include <QDBusConnection>
#include <QDBusMessage>

#include <KApplicationTrader>
#include <KQuickAddons/ConfigModule>

#include "terminal_settings.h"

ComponentChooserTerminal::ComponentChooserTerminal(QObject *parent)
    : ComponentChooser(parent,
                       QString(),
                       QStringLiteral("TerminalEmulator"),
                       QStringLiteral("org.kde.konsole.desktop"),
                       i18n("Select default terminal emulator"))
{
}

void ComponentChooserTerminal::load()
{
    m_applications.clear();

    bool preferredServiceAdded = false;

    TerminalSettings settings;
    QString preferredService = settings.terminalApplication();

    KApplicationTrader::query([&preferredServiceAdded, preferredService, this](const KService::Ptr &service) {
        if (service->exec().isEmpty() || !service->categories().contains(m_category) || service->noDisplay()) {
            return false;
        }
        QVariantMap application;
        application["name"] = service->name();
        application["icon"] = service->icon();
        application["storageId"] = service->storageId();
        application["execLine"] = service->exec();
        m_applications += application;
        if ((!preferredService.isEmpty() && preferredService == service->exec())) {
            m_index = m_applications.length() - 1;
            preferredServiceAdded = true;
        }
        if (service->storageId() == m_defaultApplication) {
            m_defaultIndex = m_applications.length() - 1;
        }
        return false;
    });
    if (!preferredService.isEmpty() && !preferredServiceAdded) {
        // standard application was specified by the user
        QVariantMap application;
        auto service = KService::serviceByStorageId(settings.terminalService());
        if (service) {
            application["name"] = service->name();
            application["icon"] = service->icon();
            application["storageId"] = service->storageId();
            application["execLine"] = service->exec();
        } else {
            application["name"] = preferredService;
            application["icon"] = QStringLiteral("application-x-shellscript");
            application["execLine"] = preferredService;
        }
        m_applications += application;
        m_index = m_applications.length() - 1;
    }
    QVariantMap application;
    application["name"] = i18n("Other…");
    application["icon"] = QStringLiteral("application-x-shellscript");
    application["storageId"] = QString();
    m_applications += application;
    if (m_index == -1) {
        m_index = 0;
    }

    m_previousApplication = m_applications[m_index].toMap()["storageId"].toString();
    Q_EMIT applicationsChanged();
    Q_EMIT indexChanged();
    Q_EMIT isDefaultsChanged();
}

void ComponentChooserTerminal::save()
{
    TerminalSettings terminalSettings;
    terminalSettings.setTerminalApplication(m_applications[m_index].toMap()["execLine"].toString());
    terminalSettings.setTerminalService(m_applications[m_index].toMap()["storageId"].toString());
    terminalSettings.save();

    QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                          QStringLiteral("/KLauncher"),
                                                          QStringLiteral("org.kde.KLauncher"),
                                                          QStringLiteral("reparseConfiguration"));
    QDBusConnection::sessionBus().send(message);

    m_previousApplication = m_applications[m_index].toMap()["storageId"].toString();
}
