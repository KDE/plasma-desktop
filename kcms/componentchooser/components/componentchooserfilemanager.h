/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "componentchooser.h"

class ComponentChooserFileManager : public ComponentChooser
{
public:
    ComponentChooserFileManager(QObject *parent);

    QStringList mimeTypes() const override;
};
