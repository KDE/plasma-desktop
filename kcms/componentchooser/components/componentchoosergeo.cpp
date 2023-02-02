/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "componentchoosergeo.h"

#include <KService>

ComponentChooserGeo::ComponentChooserGeo(QObject *parent)
    : ComponentChooser(parent, QStringLiteral("x-scheme-handler/geo"), QString(), QStringLiteral("marble_geo.desktop"), i18n("Select default map"))
{
}

static const QStringList geolocationMimetypes{"x-scheme-handler/geo"};

QStringList ComponentChooserGeo::mimeTypes() const
{
    return geolocationMimetypes;
}
