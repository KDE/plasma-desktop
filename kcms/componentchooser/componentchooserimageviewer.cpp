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
    QStringList mimetypes{"image/png", "image/jpeg", "image/webp", "image/avif", "image/heif", "image/bmp", "image/vnd.microsoft.icon", "image/x-icns"

    };
    saveMimeTypeAssociations(mimetypes, m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
