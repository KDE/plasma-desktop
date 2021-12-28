/*
    SPDX-FileCopyrightText: 2021 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERTEXTEDITOR_H
#define COMPONENTCHOOSERTEXTEDITOR_H

#include "componentchooser.h"

class ComponentChooserTextEditor : public ComponentChooser
{
public:
    ComponentChooserTextEditor(QObject *parent);

    void save() override;
};

#endif
