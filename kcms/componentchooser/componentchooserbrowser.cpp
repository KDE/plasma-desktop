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
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#include "componentchooserbrowser.h"
#include <KOpenWithDialog>
#include "browser_settings.h"

#include <KLocalizedString>
#include <KServiceTypeTrader>
#include <KApplicationTrader>

#include <QUrl>
#include <QDBusConnection>
#include <QDBusMessage>

CfgBrowser::CfgBrowser(QWidget *parent)
    : CfgPlugin(parent)
{
    connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgBrowser::selectBrowser);
}

CfgBrowser::~CfgBrowser() {
}

void CfgBrowser::selectBrowser(int index)
{
    if (index == count() -1) {
        QList<QUrl> urlList;
        KOpenWithDialog dlg(QStringLiteral("x-scheme-handler/http"), QString(), this);
        dlg.setSaveNewApplications(true);
        if (dlg.exec() != QDialog::Accepted) {
            setCurrentIndex(validLastCurrentIndex());
            return;
        }

        const auto service = dlg.service();

        // check if the selected service is already in the list
        const auto matching = model()->match(model()->index(0,0), Qt::UserRole, service->storageId());
        if (!matching.isEmpty()) {
            const int index = matching.at(0).row();
            setCurrentIndex(index);
            emit changed(index != m_currentIndex);
        } else {
            const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
            insertItem(count() -1, QIcon::fromTheme(icon), service->name(), service->storageId());
            setCurrentIndex(count() - 2);

            emit changed(true);
        }

    } else {
        emit changed(index != m_currentIndex);
    }
}

void CfgBrowser::load(KConfig *)
{
    const auto browser = KApplicationTrader::preferredService("x-scheme-handler/http");

    clear();
    m_currentIndex = -1;
    m_defaultIndex = -1;

    const auto constraint = QStringLiteral("'WebBrowser' in Categories and"
                                      " ('x-scheme-handler/http' in ServiceTypes or 'x-scheme-handler/https' in ServiceTypes)");
    const auto browsers = KServiceTypeTrader::self()->query(QStringLiteral("Application"), constraint);
    for (const auto &service : browsers) {
        addItem(QIcon::fromTheme(service->icon()), service->name(), service->storageId());

        if (browser->storageId() == service->storageId()) {
            setCurrentIndex(count() - 1);
            m_currentIndex = count() - 1;
        }
        if (service->storageId() == QStringLiteral("org.kde.falkon.desktop")) {
            m_defaultIndex = count() - 1;
        }
    }

    if (browser && m_currentIndex == -1) {
        // we have a browser specified by the user
        addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), browser->name(), browser->storageId());
        setCurrentIndex(count() - 1);
        m_currentIndex = count() - 1;
    }

    // add a other option to add a new browser
    addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), i18n("Other..."));

    emit changed(false);
}

void CfgBrowser::save(KConfig *)
{
    if (currentIndex() == count() - 1) {
        // no browser installed, nor selected
        return;
    }

    const QString browserStorageId = currentData().toString();

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

        QDBusMessage message  = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                               QStringLiteral("/KLauncher"),
                                                               QStringLiteral("org.kde.KLauncher"),
                                                               QStringLiteral("reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);

        m_currentIndex = currentIndex();

        emit changed(false);
    }
}
