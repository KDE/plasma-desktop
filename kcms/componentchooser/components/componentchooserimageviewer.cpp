/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

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

static const QStringList imageViewerMimetypes{"image/png", "image/jpeg", "image/webp", "image/avif", "image/heif", "image/bmp", "image/x-icns"};

QStringList ComponentChooserImageViewer::mimeTypes() const
{
    return imageViewerMimetypes;
}
