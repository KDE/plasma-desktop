/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERTEXTEDITOR_H
#define COMPONENTCHOOSERTEXTEDITOR_H

#include "componentchooser.h"

class ComponentChooserTextEditor : public ComponentChooser
{
public:
    ComponentChooserTextEditor(QObject *parent);

    QStringList mimeTypes() const override;
};

#endif
