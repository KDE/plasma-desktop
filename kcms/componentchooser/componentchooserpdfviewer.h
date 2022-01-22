/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERPDFVIEWER_H
#define COMPONENTCHOOSERPDFVIEWER_H

#include "componentchooser.h"

class ComponentChooserPdfViewer : public ComponentChooser
{
public:
    ComponentChooserPdfViewer(QObject *parent);

    void save() override;
};

#endif
