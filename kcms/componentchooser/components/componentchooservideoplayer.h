/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERVIDEOPLAYER_H
#define COMPONENTCHOOSERVIDEOPLAYER_H

#include "componentchooser.h"

class ComponentChooserVideoPlayer : public ComponentChooser
{
public:
    ComponentChooserVideoPlayer(QObject *parent);

    QStringList mimeTypes() const override;
};

#endif
