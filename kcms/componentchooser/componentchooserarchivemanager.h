/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERARCHIVEMANAGER_H
#define COMPONENTCHOOSERARCHIVEMANAGER_H

#include "componentchooser.h"

class ComponentChooserArchiveManager : public ComponentChooser
{
public:
    ComponentChooserArchiveManager(QObject *parent);

    void save() override;
};

#endif
