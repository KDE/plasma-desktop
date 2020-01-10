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

#include <kemailsettings.h>
#include <kopenwithdialog.h>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KService>
#include <KServiceTypeTrader>
#include <KBuildSycocaProgressDialog>
#include <KMimeTypeTrader>
#include <KShell>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QUrl>
#include <QFile>
#include <QCheckBox>

namespace {
    static const char s_AddedAssociations[] = "Added Associations";
    static const auto s_mimetype = QStringLiteral("x-scheme-handler/mailto");
}

CfgEmailClient::CfgEmailClient(QWidget *parent)
    : QWidget(parent), Ui::EmailClientConfig_UI(), CfgPlugin()
{
    setupUi( this );

    pSettings = new KEMailSettings();

    connect(emailClientsCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgEmailClient::selectEmailClient);
}

CfgEmailClient::~CfgEmailClient() {
    delete pSettings;
}

void CfgEmailClient::defaults()
{
    // select kmail if installed
    if (m_kmailIndex != -1) {
        emailClientsCombo->setCurrentIndex(m_kmailIndex);
    }
}

bool CfgEmailClient::isDefaults() const
{
    // if kmail is installed and is selected
    if (m_kmailIndex != -1) {
        return emailClientsCombo->currentIndex() == m_kmailIndex;
    }

    return true;
}

void CfgEmailClient::load(KConfig *)
{
    const KService::Ptr emailClientService = KMimeTypeTrader::self()->preferredService(s_mimetype);

    const auto emailClients = KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                                QStringLiteral("'Email' in Categories and 'x-scheme-handler/mailto' in ServiceTypes"));

    emailClientsCombo->clear();
    m_kmailIndex = -1;
    m_currentIndex = -1;

    for (const auto &service : emailClients) {

        emailClientsCombo->addItem(QIcon::fromTheme(service->icon()), service->name(), service->storageId());

        if (emailClientService && emailClientService->storageId() == service->storageId()) {
            emailClientsCombo->setCurrentIndex(emailClientsCombo->count() - 1);
            m_currentIndex = emailClientsCombo->count() - 1;
        }
        if (service->storageId() == QStringLiteral("org.kde.kmail2.desktop") ||
                service->storageId() == QStringLiteral("org.kde.kmail.desktop")) {
            m_kmailIndex = emailClientsCombo->count() - 1;
        }
    }

    // add the Added association to x-scheme-handler/mailto from the mimeapps.list file
    const KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    const KConfigGroup addedApps(profile, s_AddedAssociations);
    const auto addedList = addedApps.readXdgListEntry(s_mimetype);

    for (const auto &addedApp : addedList) {
        // without .desktop extension
        auto service = KService::serviceByStorageId(addedApp.mid(0, addedApp.length() -8));
        if (!service) {
            service = KService::serviceByStorageId(addedApp);
        }
        if (!service) {
            continue;
        }
        // avoid duplicates entry when email clients are present in mimeapps.list's Added Associations too
        const bool isServiceAlreadyInserted = std::none_of(emailClients.constBegin(), emailClients.constEnd(), [service] (const KService::Ptr &serv) { return service->storageId() == serv->storageId(); });
        if (isServiceAlreadyInserted) {
            const auto icon = QIcon::fromTheme(!service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript"));
            emailClientsCombo->addItem(icon, service->name() + " (" + KShell::tildeCollapse(service->entryPath()) + ")", service->storageId());

            if (emailClientService && emailClientService->storageId() == service->storageId()) {
                emailClientsCombo->setCurrentIndex(emailClientsCombo->count() - 1);
                m_currentIndex = emailClientsCombo->count() - 1;
            }
        }
    }

    // add a other option to add a new email client with KOpenWithDialog
    emailClientsCombo->addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), i18n("Other..."), QStringLiteral());

    emit changed(false);
}

void CfgEmailClient::selectEmailClient(int index)
{
    if (index == emailClientsCombo->count() -1) {
        // Other option

        KOpenWithDialog dlg(s_mimetype, QString(), this);
        dlg.setSaveNewApplications(true);

        if (dlg.exec() != QDialog::Accepted) {
            // restore previous setting
            emailClientsCombo->setCurrentIndex(m_currentIndex);
            emit changed(false);
        } else {
            const auto service = dlg.service();

            const auto icon = QIcon::fromTheme(!service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript"));
            emailClientsCombo->insertItem(emailClientsCombo->count() - 1, icon, service->name() + " (" + KShell::tildeCollapse(service->entryPath()) + ")", service->storageId());

            // select newly inserted email client
            emailClientsCombo->setCurrentIndex(emailClientsCombo->count() - 2);

            emit changed(true);
            return;
        }
    } else {
        emit changed(m_currentIndex != index);
    }
}

void CfgEmailClient::save(KConfig *)
{
    const QString &storageId = emailClientsCombo->currentData().toString();
    const KService::Ptr emailClientService = KService::serviceByStorageId(storageId);

    const bool kmailSelected = m_kmailIndex != -1 && emailClientsCombo->currentIndex() == m_kmailIndex;
    if (kmailSelected) {
        pSettings->setSetting(KEMailSettings::ClientProgram, QString());
        pSettings->setSetting(KEMailSettings::ClientTerminal, QStringLiteral("false"));
    } else {
        pSettings->setSetting(KEMailSettings::ClientProgram, emailClientService->storageId());
        pSettings->setSetting(KEMailSettings::ClientTerminal, emailClientService->terminal() ? QStringLiteral("true") : QStringLiteral("false"));
    }

    // Save the default email client in mimeapps.list
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (profile->isConfigWritable(true) && emailClientService) {

        KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);

        // Save the default application according to mime-apps-spec 1.0
        KConfigGroup defaultApp(profile, "Default Applications");
        defaultApp.writeXdgListEntry(s_mimetype, {emailClientService->storageId()});

        KConfigGroup addedApps(profile, "Added Associations");
        QStringList apps = addedApps.readXdgListEntry(s_mimetype);
        apps.removeAll(emailClientService->storageId());
        apps.prepend(emailClientService->storageId()); // make it the preferred app, i.e first in list
        addedApps.writeXdgListEntry(s_mimetype, apps);

        profile->sync();

        m_currentIndex = emailClientsCombo->currentIndex();

        // refresh cache
        KBuildSycocaProgressDialog::rebuildKSycoca(this);

        emit changed(false);
    }
}

