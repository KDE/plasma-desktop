/*
    SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "activitysettings.h"

#include <QMessageBox>
#include <QProcess>

#include <KAuthorized>
#include <KLocalizedString>

#include "dialog.h"

#include <kactivities/controller.h>
#include <kactivities/info.h>

ActivitySettings::ActivitySettings(QObject *parent)
    : QObject(parent)
    , m_newActivityAuthorized(KAuthorized::authorize(QStringLiteral("plasma-desktop/add_activities")))
{
}

ActivitySettings::~ActivitySettings()
{
}

bool ActivitySettings::newActivityAuthorized() const
{
    return m_newActivityAuthorized;
}

void ActivitySettings::configureActivity(const QString &id)
{
    Dialog::showDialog(id);
}

void ActivitySettings::newActivity()
{
    Dialog::showDialog();
}

void ActivitySettings::deleteActivity(const QString &id)
{
    QMetaObject::invokeMethod(
        this,
        [id] {
            KActivities::Info info(id);

            if (QMessageBox::question(nullptr, i18nc("@title:window", "Delete Activity"), i18n("Are you sure you want to delete '%1'?", info.name()))
                == QMessageBox::Yes) {
                KActivities::Controller().removeActivity(id);
            }
        },
        Qt::ConnectionType::QueuedConnection);
}

void ActivitySettings::configureActivities()
{
    QProcess::startDetached(QStringLiteral("kcmshell5"), {QStringLiteral("activities")});
}
