/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "componentchooser.h"

class ComponentChooserEmail : public ComponentChooser
{
public:
    ComponentChooserEmail(QObject *parent);

    QStringList mimeTypes() const override;
    void save() override;
};
