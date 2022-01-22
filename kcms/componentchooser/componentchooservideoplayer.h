/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERVIDEOPLAYER_H
#define COMPONENTCHOOSERVIDEOPLAYER_H

#include "componentchooser.h"

class ComponentChooserVideoPlayer : public ComponentChooser
{
public:
    ComponentChooserVideoPlayer(QObject *parent);

    void save() override;
};

#endif
