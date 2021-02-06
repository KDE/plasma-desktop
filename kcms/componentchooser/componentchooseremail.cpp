/***************************************************************************
 *   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>                  *
 *   Copyright (C) 2020 Méven Car <meven.car@kdemail.net>                  *
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "componentchooseremail.h"

#include <KEMailSettings>
#include <KService>

ComponentChooserEmail::ComponentChooserEmail(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("x-scheme-handler/mailto"),
                       QStringLiteral("Email"),
                       QStringLiteral("org.kde.kmail2.desktop"),
                       i18n("Select default e-mail client"))
{
}

void ComponentChooserEmail::save()
{
    const QString storageId = m_applications[m_index].toMap()["storageId"].toString();
    const KService::Ptr emailClientService = KService::serviceByStorageId(storageId);
    if (!emailClientService) {
        return;
    }
    const bool kmailSelected = storageId == QStringLiteral("org.kde.kmail2.desktop");

    KEMailSettings *emailSettings = new KEMailSettings();

    if (kmailSelected) {
        emailSettings->setSetting(KEMailSettings::ClientProgram, QString());
        emailSettings->setSetting(KEMailSettings::ClientTerminal, QStringLiteral("false"));
    } else {
        emailSettings->setSetting(KEMailSettings::ClientProgram, storageId);
        emailSettings->setSetting(KEMailSettings::ClientTerminal, emailClientService->terminal() ? QStringLiteral("true") : QStringLiteral("false"));
    }

    delete emailSettings;

    saveMimeTypeAssociation(QStringLiteral("x-scheme-handler/mailto"), storageId);
}
