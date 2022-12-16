/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "componentchooser.h"

class ComponentChooserBrowser : public ComponentChooser
{
public:
    ComponentChooserBrowser(QObject *parent);

    void save() override;
};
