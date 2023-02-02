/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERMUSICPLAYER_H
#define COMPONENTCHOOSERMUSICPLAYER_H

#include "componentchooser.h"

class ComponentChooserMusicPlayer : public ComponentChooser
{
public:
    ComponentChooserMusicPlayer(QObject *parent);

    QStringList mimeTypes() const override;
};

#endif
