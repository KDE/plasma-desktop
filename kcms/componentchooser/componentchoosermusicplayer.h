/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERMUSICPLAYER_H
#define COMPONENTCHOOSERMUSICPLAYER_H

#include "componentchooser.h"

class ComponentChooserMusicPlayer : public ComponentChooser
{
public:
    ComponentChooserMusicPlayer(QObject *parent);

    void save() override;
};

#endif
