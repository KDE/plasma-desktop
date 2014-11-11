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
#include <kdebug.h>
#include <kprocess.h>
#include <kmimetypetrader.h>
#include <kopenwithdialog.h>
#include <kconfiggroup.h>
#include <QStandardPaths>

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
    load(0);
}

static KService::List appOffers()
{
    return KMimeTypeTrader::self()->query("inode/directory", "Application");
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

void CfgFileManager::save(KConfig *)
{
    QString storageId;
    Q_FOREACH(QRadioButton* button, qFindChildren<QRadioButton*>(this)) {
        if (button->isChecked()) {
            storageId = button->property("storageId").toString();
        }
    }

    kDebug() << storageId;
    if (!storageId.isEmpty()) {
        // This is taken from filetypes/mimetypedata.cpp
        KSharedConfig::Ptr profile = KSharedConfig::openConfig("mimeapps.list", KConfig::NoGlobals, QStandardPaths::ApplicationsLocation);
        if (!profile->isConfigWritable(true)) // warn user if mimeapps.list is root-owned (#155126/#94504)
            return;
        KConfigGroup addedApps(profile, "Added Associations");
        QStringList userApps = addedApps.readXdgListEntry("inode/directory");
        userApps.removeAll(storageId); // remove if present, to make it first in the list
        userApps.prepend(storageId);
        addedApps.writeXdgListEntry("inode/directory", userApps);

        Kdelibs4SharedConfig::syncConfigGroup(&addedApps, "mimeapps.list");

        profile->sync();
        KBuildSycocaProgressDialog::rebuildKSycoca(this);
    }

    emit changed(false);
}

void CfgFileManager::slotAddFileManager()
{
    KProcess proc;
    proc << "keditfiletype5";
    proc << "inode/directory";
    if (proc.execute() == 0) {
        load(0);
    }
}
