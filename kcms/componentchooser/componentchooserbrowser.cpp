/***************************************************************************
                          componentchooserbrowser.cpp
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger <jowenn@kde.org>
    copyright            : (C) 2020 by Méven Car <meven.car@enioka.com>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundationi                            *
 *                                                                         *
 ***************************************************************************/

#include "componentchooserbrowser.h"
#include <kopenwithdialog.h>
#include "browser_settings.h"

#include <KBuildSycocaProgressDialog>
#include <KLocalizedString>
#include <KServiceTypeTrader>
#include <KMimeTypeTrader>

#include <QUrl>
#include <QDBusConnection>
#include <QDBusMessage>

CfgBrowser::CfgBrowser(QWidget *parent)
    : QWidget(parent), Ui::BrowserConfig_UI(),CfgPlugin()
{
    setupUi(this);
    connect(browserCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgBrowser::selectBrowser);

}

CfgBrowser::~CfgBrowser() {
}

void CfgBrowser::selectBrowser(int index)
{
    if (index == browserCombo->count() -1) {
        QList<QUrl> urlList;
        KOpenWithDialog dlg(QStringLiteral("x-scheme-handler/http"), QString(), this);
        dlg.setSaveNewApplications(true);
        if (dlg.exec() != QDialog::Accepted) {
            browserCombo->setCurrentIndex(m_currentIndex);
            return;
        }

        const auto service = dlg.service();

        // check if the selected service is already in the list
        const auto matching = browserCombo->model()->match(browserCombo->model()->index(0,0), Qt::UserRole, service->storageId());
        if (!matching.isEmpty()) {
            const int index = matching.at(0).row();
            browserCombo->setCurrentIndex(index);
            emit changed(index != m_currentIndex);
        } else {
            const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
            browserCombo->insertItem(browserCombo->count() -1, QIcon::fromTheme(icon), service->name(), service->storageId());
            browserCombo->setCurrentIndex(browserCombo->count() - 2);

            emit changed(true);
        }

    } else {
        emit changed(index != m_currentIndex);
    }
}

void CfgBrowser::defaults()
{
    if (m_falkonIndex != -1) {
        browserCombo->setCurrentIndex(m_falkonIndex);
    }
}

bool CfgBrowser::isDefaults() const
{
    return m_falkonIndex == -1 || m_falkonIndex == browserCombo->currentIndex();
}

void CfgBrowser::load(KConfig *) 
{
    const auto browser = KMimeTypeTrader::self()->preferredService("x-scheme-handler/http");

    browserCombo->clear();
    m_currentIndex = -1;
    m_falkonIndex = -1;

    const auto constraint = QStringLiteral("'WebBrowser' in Categories and"
                                      " ('x-scheme-handler/http' in ServiceTypes or 'x-scheme-handler/https' in ServiceTypes)");
    const auto browsers = KServiceTypeTrader::self()->query(QStringLiteral("Application"), constraint);
    for (const auto &service : browsers) {
        browserCombo->addItem(QIcon::fromTheme(service->icon()), service->name(), service->storageId());

        if (browser->storageId() == service->storageId()) {
            browserCombo->setCurrentIndex(browserCombo->count() - 1);
            m_currentIndex = browserCombo->count() - 1;
        }
        if (service->storageId() == QStringLiteral("org.kde.falkon.desktop")) {
            m_falkonIndex = browserCombo->count() - 1;
        }
    }

    if (browser && m_currentIndex == -1) {
        // we have a browser specified by the user
        browserCombo->addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), browser->name(), browser->storageId());
        browserCombo->setCurrentIndex(browserCombo->count() - 1);
        m_currentIndex = browserCombo->count() - 1;
    }

    // add a other option to add a new browser
    browserCombo->addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), i18n("Other..."), QStringLiteral());

    emit changed(false);
}

void CfgBrowser::save(KConfig *)
{
    const QString browserStorageId = browserCombo->currentData().toString();

    BrowserSettings settings;
    settings.setBrowserApplication(browserStorageId);
    settings.save();

    // Save the default browser as scheme handler for http(s) in mimeapps.list
    KSharedConfig::Ptr mimeAppList = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (mimeAppList->isConfigWritable(true /*warn user if not writable*/)) {
        KConfigGroup defaultApp(mimeAppList, "Default Applications");
        defaultApp.writeXdgListEntry(QStringLiteral("x-scheme-handler/http"), QStringList(browserStorageId));
        defaultApp.writeXdgListEntry(QStringLiteral("x-scheme-handler/https"), QStringList(browserStorageId));
        mimeAppList->sync();

        KBuildSycocaProgressDialog::rebuildKSycoca(this);

        QDBusMessage message  = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                               QStringLiteral("/KLauncher"),
                                                               QStringLiteral("org.kde.KLauncher"),
                                                               QStringLiteral("reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);

        m_currentIndex = browserCombo->currentIndex();

        emit changed(false);
    }
}
