/*  This file is part of the KDE project
    Copyright (C) 2008 David Faure <faure@kde.org>

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
#include <kbuildsycocaprogressdialog.h>
#include <QDebug>
#include <kprocess.h>
#include <kmimetypetrader.h>
#include <kopenwithdialog.h>
#include <kconfiggroup.h>
#include <QStandardPaths>
#include <QFileInfo>

#include "../migrationlib/kdelibs4config.h"

CfgFileManager::CfgFileManager(QWidget *parent)
    : QWidget(parent), Ui::FileManagerConfig_UI(),CfgPlugin()
{
    setupUi(this);
    connect(btnSelectFileManager, &QToolButton::clicked, this, &CfgFileManager::slotAddFileManager);
}

CfgFileManager::~CfgFileManager() {
}

void CfgFileManager::configChanged()
{
    emit changed(true);
}

void CfgFileManager::defaults()
{
    load(nullptr);
}

static KService::List appOffers()
{
    return KMimeTypeTrader::self()->query(QStringLiteral("inode/directory"), QStringLiteral("Application"));
}

void CfgFileManager::load(KConfig *) {
    qDeleteAll(mDynamicWidgets);
    mDynamicWidgets.clear();
    const KService::List apps = appOffers();
    bool first = true;
    Q_FOREACH(const KService::Ptr& service, apps)
    {
        QRadioButton* button = new QRadioButton(service->name(), this);
        connect(button, &QRadioButton::toggled, this, &CfgFileManager::configChanged);
        button->setProperty("storageId", service->storageId());
        radioLayout->addWidget(button);
        if (first) {
            button->setChecked(true);
            first = false;
        }
        mDynamicWidgets << button;
    }

    emit changed(false);
}

static const char s_DefaultApplications[] = "Default Applications";
static const char s_AddedAssociations[] = "Added Associations";
static const char s_RemovedAssociations[] = "Removed Associations";

void CfgFileManager::save(KConfig *)
{
    QString storageId;
    Q_FOREACH(QRadioButton* button, qFindChildren<QRadioButton*>(this)) {
        if (button->isChecked()) {
            storageId = button->property("storageId").toString();
        }
    }

    qDebug() << storageId;
    if (!storageId.isEmpty()) {
        // This is taken from filetypes/mimetypedata.cpp
        KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
        if (!profile->isConfigWritable(true)) // warn user if mimeapps.list is root-owned (#155126/#94504)
            return;
        const QString mime = QStringLiteral("inode/directory");
        KConfigGroup addedApps(profile, s_AddedAssociations);
        QStringList userApps = addedApps.readXdgListEntry(mime);
        userApps.removeAll(storageId); // remove if present, to make it first in the list
        userApps.prepend(storageId);
        addedApps.writeXdgListEntry(mime, userApps);

        // Save the default file manager as per mime-apps spec 1.0.1
        KConfigGroup defaultApp(profile, s_DefaultApplications);
        defaultApp.writeXdgListEntry(mime, QStringList(storageId));

        Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("Added Associations"), QStringLiteral("mimeapps.list"));

        profile->sync();

        // Clean out any kde-mimeapps.list which would take precedence any cancel our changes.
        // (also taken from filetypes/mimetypedata.cpp)
        const QString desktops = QString::fromLocal8Bit(qgetenv("XDG_CURRENT_DESKTOP"));
        foreach (const QString &desktop, desktops.split(":", QString::SkipEmptyParts)) {
            const QString file = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                + QLatin1Char('/') + desktop.toLower() + QLatin1String("-mimeapps.list");
            if (QFileInfo::exists(file)) {
                qDebug() << "Cleaning up" << file;
                KConfig conf(file, KConfig::NoGlobals);
                KConfigGroup(&conf, s_DefaultApplications).deleteEntry(mime);
                KConfigGroup(&conf, s_AddedAssociations).deleteEntry(mime);
                KConfigGroup(&conf, s_RemovedAssociations).deleteEntry(mime);
            }
        }

        KBuildSycocaProgressDialog::rebuildKSycoca(this);
    }

    emit changed(false);
}

void CfgFileManager::slotAddFileManager()
{
    KProcess proc;
    proc << QStringLiteral("keditfiletype5");
    proc << QStringLiteral("inode/directory");
    if (proc.execute() == 0) {
        load(nullptr);
    }
}
