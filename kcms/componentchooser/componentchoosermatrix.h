/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERMATRIX_H
#define COMPONENTCHOOSERMATRIX_H

#include "componentchooser.h"

class ComponentChooserMatrix : public ComponentChooser
{
public:
    ComponentChooserMatrix(QObject *parent);

    void save() override;
};

#endif
