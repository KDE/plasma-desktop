/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "componentchoosertel.h"

#include <KService>

ComponentChooserTel::ComponentChooserTel(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("x-scheme-handler/tel"),
                       QStringLiteral("Dialer"),
                       QStringLiteral("org.kde.kdeconnect.handler.desktop"),
                       i18n("Select default dialer application"))
{
}

static const QStringList telMimetypes{"x-scheme-handler/tel"};

QStringList ComponentChooserTel::mimeTypes() const
{
    return telMimetypes;
}
