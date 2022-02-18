/*
 *    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
 *    SPDX-FileCopyrightText: 2020 Méven Car <meven.car@kdemail.net>
 *    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

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
    const QString storageId = m_applications[m_index].toMap()[QStringLiteral("storageId")].toString();
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

    saveMimeTypeAssociations(QStringList("x-scheme-handler/mailto"), storageId);
}
