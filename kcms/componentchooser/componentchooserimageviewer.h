/*
    SPDX-FileCopyrightText: 2021 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERIMAGEVIEWER_H
#define COMPONENTCHOOSERIMAGEVIEWER_H

#include "componentchooser.h"

class ComponentChooserImageViewer : public ComponentChooser
{
public:
    ComponentChooserImageViewer(QObject *parent);

    void save() override;
};

#endif
