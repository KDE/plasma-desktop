/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserimageviewer.h"

ComponentChooserImageViewer::ComponentChooserImageViewer(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("image/png"),
                       QStringLiteral("Viewer"),
                       QStringLiteral("org.kde.gwenview.desktop"),
                       i18n("Select default image viewer"))
{
}

void ComponentChooserImageViewer::save()
{
    saveMimeTypeAssociation(QStringLiteral("image/png"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("image/jpeg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("image/webp"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("image/avif"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("image/heif"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("image/bmp"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("image/vnd.microsoft.icon"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("image/x-icns"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
