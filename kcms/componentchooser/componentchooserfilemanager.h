/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERFILEMANAGER_H
#define COMPONENTCHOOSERFILEMANAGER_H

#include "componentchooser.h"

class ComponentChooserFileManager : public ComponentChooser
{
public:
    ComponentChooserFileManager(QObject *parent);

    void save() override;
};

#endif
