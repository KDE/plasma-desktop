/*
 *    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
 *    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "componentchoosergeo.h"

#include <KService>

ComponentChooserGeo::ComponentChooserGeo(QObject *parent)
    : ComponentChooser(parent, QStringLiteral("x-scheme-handler/geo"), QString(), QStringLiteral("marble_geo.desktop"), i18n("Select default map"))
{
}

void ComponentChooserGeo::save()
{
    const QString storageId = m_applications[m_index].toMap()[QStringLiteral("storageId")].toString();
    const KService::Ptr geoClientService = KService::serviceByStorageId(storageId);
    if (!geoClientService) {
        return;
    }

    saveMimeTypeAssociations(QStringList("x-scheme-handler/geo"), storageId);
}
