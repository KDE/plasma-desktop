/*
 *   Copyright (C) 2015 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "activitysettings.h"

#include <QMessageBox>
#include <QProcess>

#include <KLocalizedString>

#include "dialog.h"

#include <kactivities/info.h>
#include <kactivities/controller.h>

ActivitySettings::ActivitySettings(QObject *parent)
    : QObject(parent)
{
}

ActivitySettings::~ActivitySettings()
{
}

void ActivitySettings::configureActivity(const QString &id)
{
    auto dialog = new Dialog(id);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ActivitySettings::newActivity()
{
    auto dialog = new Dialog();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void ActivitySettings::deleteActivity(const QString &id)
{
    KActivities::Info info(id);

    if (QMessageBox::question(Q_NULLPTR, i18n("Delete activity"),
                              i18n("Are you sure you want to delete '%1'?",
                                   info.name())) == QMessageBox::Yes) {
        KActivities::Controller().removeActivity(id);
    }
}

void ActivitySettings::configureActivities()
{
    QProcess::startDetached("kcmshell5", { "activities" });
}


#include "activitysettings.moc"


