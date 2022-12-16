/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "componentchooser.h"

class ComponentChooserTerminal : public ComponentChooser
{
public:
    ComponentChooserTerminal(QObject *parent);

    void load() override;
    void save() override;
};
