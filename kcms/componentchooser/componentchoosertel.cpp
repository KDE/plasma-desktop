/*
      SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
      SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "componentchoosertel.h"

#include <KService>

ComponentChooserTel::ComponentChooserTel(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("x-scheme-handler/tel"),
                       QString(),
                       QStringLiteral("org.kde.kdeconnect.handler.desktop"),
                       i18n("Select default dialer application"))
{
}

void ComponentChooserTel::save()
{
    const QString storageId = m_applications[m_index].toMap()[QStringLiteral("storageId")].toString();
    const KService::Ptr telClientService = KService::serviceByStorageId(storageId);
    if (!telClientService) {
        return;
    }

    saveMimeTypeAssociations({QStringLiteral("x-scheme-handler/tel")}, storageId);
}
