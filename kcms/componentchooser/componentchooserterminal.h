/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERTERMINAL_H
#define COMPONENTCHOOSERTERMINAL_H

#include "componentchooser.h"

class ComponentChooserTerminal : public ComponentChooser
{
public:
    ComponentChooserTerminal(QObject *parent);

    void load() override;
    void save() override;
};

#endif
