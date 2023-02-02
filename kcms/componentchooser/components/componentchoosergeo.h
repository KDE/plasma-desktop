/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "componentchooser.h"

class ComponentChooserGeo : public ComponentChooser
{
public:
    explicit ComponentChooserGeo(QObject *parent);

    QStringList mimeTypes() const override;
};
