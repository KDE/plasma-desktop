/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERPDFVIEWER_H
#define COMPONENTCHOOSERPDFVIEWER_H

#include "componentchooser.h"

class ComponentChooserPdfViewer : public ComponentChooser
{
public:
    ComponentChooserPdfViewer(QObject *parent);

    QStringList mimeTypes() const override;
};

#endif
