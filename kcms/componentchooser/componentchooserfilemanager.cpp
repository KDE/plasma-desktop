/*  This file is part of the KDE project
    Copyright (C) 2008 David Faure <faure@kde.org>
    Copyright (C) 2020 Méven Car <meven.car@enioka.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License or ( at
    your option ) version 3 or, at the discretion of KDE e.V. ( which shall
    act as a proxy as in section 14 of the GPLv3 ), any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; see the file COPYING.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "componentchooserfilemanager.h"
#include <KProcess>
#include <KApplicationTrader>
#include <KOpenWithDialog>
#include <KConfigGroup>
#include <QStandardPaths>
#include <KSharedConfig>

CfgFileManager::CfgFileManager(QWidget *parent)
    : CfgPlugin(parent)
{
    connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgFileManager::selectFileManager);
}

CfgFileManager::~CfgFileManager() {
}

void CfgFileManager::selectFileManager(int index)
{
    if (index == count() -1) {

        KOpenWithDialog dlg({}, i18n("Select preferred file manager:"), QString(), this);
        dlg.setSaveNewApplications(true);
        if (dlg.exec() != QDialog::Accepted) {
            setCurrentIndex(validLastCurrentIndex());
            return;
        }

        const auto service = dlg.service();

        // if the selected service is already in the list
        const auto matching = model()->match(model()->index(0,0), Qt::UserRole, service->storageId());
        if (!matching.isEmpty()) {
            const int index = matching.at(0).row();
            setCurrentIndex(index);
            changed(index != m_currentIndex);
        } else {
            const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
            insertItem(count() -1, QIcon::fromTheme(icon), service->name(), service->storageId());
            setCurrentIndex(count() - 2);

            changed(true);
        }
    } else {
        changed(index != m_currentIndex);
    }
}

static const QString mime = QStringLiteral("inode/directory");

void CfgFileManager::load(KConfig *)
{
    clear();
    m_currentIndex = -1;
    m_defaultIndex = -1;

    const KService::Ptr fileManager = KApplicationTrader::preferredService(mime);

    const auto fileManagers = KApplicationTrader::query([] (const KService::Ptr &service) {
        if (service->exec().isEmpty()) {
            return false;
        }
        return service->categories().contains("FileManager");
    });
    for (const KService::Ptr &service : fileManagers) {
        addItem(QIcon::fromTheme(service->icon()), service->name(), service->storageId());

        if (fileManager->storageId() == service->storageId()) {
            setCurrentIndex(count() -1);
            m_currentIndex = count() -1;
        }
        if (service->storageId() == QStringLiteral("org.kde.dolphin.desktop")) {
            m_defaultIndex = count() -1;
        }
    }

    // in case of a service not associated with FileManager Category
    if (m_currentIndex == -1 && !fileManager->storageId().isEmpty()) {
        const KService::Ptr service = KService::serviceByStorageId(fileManager->storageId());

        const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
        addItem(QIcon::fromTheme(icon), service->name(), service->storageId());
        setCurrentIndex(count() -1);
        m_currentIndex = count() -1;
    }

    // add a other option to add a new file manager with KOpenWithDialog
    addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), i18n("Other..."), QStringLiteral());

    emit changed(false);
}

static const char s_DefaultApplications[] = "Default Applications";
static const char s_AddedAssociations[] = "Added Associations";

void CfgFileManager::save(KConfig *)
{
    if (currentIndex() == count() - 1) {
        // no filemanager installed, nor selected
        return;
    }

    const QString storageId = currentData().toString();
    m_currentIndex = currentIndex();

    // This is taken from filetypes/mimetypedata.cpp
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (!profile->isConfigWritable(true)) // warn user if mimeapps.list is root-owned (#155126/#94504)
        return;
    KConfigGroup addedApps(profile, s_AddedAssociations);
    QStringList userApps = addedApps.readXdgListEntry(mime);
    userApps.removeAll(storageId); // remove if present, to make it first in the list
    userApps.prepend(storageId);
    addedApps.writeXdgListEntry(mime, userApps);

    // Save the default file manager as per mime-apps spec 1.0.1
    KConfigGroup defaultApp(profile, s_DefaultApplications);
    defaultApp.writeXdgListEntry(mime, QStringList(storageId));

    profile->sync();

    emit changed(false);
}
