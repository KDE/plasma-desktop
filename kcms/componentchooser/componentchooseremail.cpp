/***************************************************************************
                          componentchooseremail.cpp
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger <jowenn@kde.org>
    copyright            : (C) 2020 by Méven Car <meven.car@enioka.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#include "componentchooseremail.h"

#include <KEMailSettings>
#include <KOpenWithDialog>

#include <KApplicationTrader>
#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KService>
#include <KServiceTypeTrader>
#include <KShell>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QUrl>
#include <QFile>
#include <QCheckBox>

namespace {
    static const char s_defaultApplication[] = "Default Applications";
    static const char s_AddedAssociations[] = "Added Associations";
    static const auto s_mimetype = QStringLiteral("x-scheme-handler/mailto");
}

CfgEmailClient::CfgEmailClient(QWidget *parent)
    : CfgPlugin(parent)
{
    pSettings = new KEMailSettings();

    connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgEmailClient::selectEmailClient);
}

CfgEmailClient::~CfgEmailClient() {
    delete pSettings;
}

void CfgEmailClient::load(KConfig *)
{
    const KService::Ptr emailClientService = KApplicationTrader::preferredService(s_mimetype);

    const auto emailClients = KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                                QStringLiteral("'Email' in Categories and 'x-scheme-handler/mailto' in ServiceTypes"));

    clear();
    m_currentIndex = -1;
    m_defaultIndex = -1;

    for (const auto &service : emailClients) {

        addItem(QIcon::fromTheme(service->icon()), service->name(), service->storageId());

        if (emailClientService && emailClientService->storageId() == service->storageId()) {
            setCurrentIndex(count() - 1);
            m_currentIndex = count() - 1;
        }
        if (service->storageId() == QStringLiteral("org.kde.kmail2.desktop") ||
                service->storageId() == QStringLiteral("org.kde.kmail.desktop")) {
            m_defaultIndex = count() - 1;
        }
    }

    // in case of a service not associated with Email Category and/or x-scheme-handler/mailto
    if (m_currentIndex == -1 && emailClientService && !emailClientService->storageId().isEmpty()) {
        const KService::Ptr service = KService::serviceByStorageId(emailClientService->storageId());

        const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
        addItem(QIcon::fromTheme(icon), service->name(), service->storageId());
        setCurrentIndex(count() - 1);
        m_currentIndex = count() - 1;
    }

    // add a other option to add a new email client with KOpenWithDialog
    addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), i18n("Other..."), QStringLiteral());

    emit changed(false);
}

void CfgEmailClient::selectEmailClient(int index)
{
    if (index == count() -1) {
        // Other option

        KOpenWithDialog dlg(s_mimetype, QString(), this);
        dlg.setSaveNewApplications(true);

        if (dlg.exec() != QDialog::Accepted) {
            // restore previous setting
            setCurrentIndex(validLastCurrentIndex());
            emit changed(false);
        } else {
            const auto service = dlg.service();

            const auto icon = QIcon::fromTheme(!service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript"));
            insertItem(count() - 1, icon, service->name() + " (" + KShell::tildeCollapse(service->entryPath()) + ")", service->storageId());

            // select newly inserted email client
            setCurrentIndex(count() - 2);

            emit changed(true);
            return;
        }
    } else {
        emit changed(m_currentIndex != index);
    }
}

void CfgEmailClient::save(KConfig *)
{
    if (currentIndex() == count() - 1) {
        // no email client installed, nor selected
        return;
    }

    const QString &storageId = currentData().toString();
    const KService::Ptr emailClientService = KService::serviceByStorageId(storageId);
    if (!emailClientService) {
        // double checking, the selected email client might have been removed
        return;
    }

    const bool kmailSelected = m_defaultIndex != -1 && currentIndex() == m_defaultIndex;
    if (kmailSelected) {
        pSettings->setSetting(KEMailSettings::ClientProgram, QString());
        pSettings->setSetting(KEMailSettings::ClientTerminal, QStringLiteral("false"));
    } else {
        pSettings->setSetting(KEMailSettings::ClientProgram, emailClientService->storageId());
        pSettings->setSetting(KEMailSettings::ClientTerminal, emailClientService->terminal() ? QStringLiteral("true") : QStringLiteral("false"));
    }

    // Save the default email client in mimeapps.list
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (profile->isConfigWritable(true)) {

        KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);

        // Save the default application according to mime-apps-spec 1.0
        KConfigGroup defaultApp(profile, s_defaultApplication);
        defaultApp.writeXdgListEntry(s_mimetype, {emailClientService->storageId()});

        KConfigGroup addedApps(profile, s_AddedAssociations);
        QStringList apps = addedApps.readXdgListEntry(s_mimetype);
        apps.removeAll(emailClientService->storageId());
        apps.prepend(emailClientService->storageId()); // make it the preferred app, i.e first in list
        addedApps.writeXdgListEntry(s_mimetype, apps);

        profile->sync();

        m_currentIndex = currentIndex();

        emit changed(false);
    }
}

