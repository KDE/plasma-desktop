/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERARCHIVEMANAGER_H
#define COMPONENTCHOOSERARCHIVEMANAGER_H

#include "componentchooser.h"

class ComponentChooserArchiveManager : public ComponentChooser
{
public:
    ComponentChooserArchiveManager(QObject *parent);

    QStringList mimeTypes() const override;
};

#endif
