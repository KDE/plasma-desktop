/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERIMAGEVIEWER_H
#define COMPONENTCHOOSERIMAGEVIEWER_H

#include "componentchooser.h"

class ComponentChooserImageViewer : public ComponentChooser
{
public:
    ComponentChooserImageViewer(QObject *parent);

    QStringList mimeTypes() const override;
};

#endif
