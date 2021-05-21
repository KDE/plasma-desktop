/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSEREMAIL_H
#define COMPONENTCHOOSEREMAIL_H

#include "componentchooser.h"

class ComponentChooserEmail : public ComponentChooser
{
public:
    ComponentChooserEmail(QObject *parent);

    void save() override;
};

#endif
