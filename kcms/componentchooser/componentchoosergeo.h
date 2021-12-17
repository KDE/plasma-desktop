/*
    SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERGEO_H
#define COMPONENTCHOOSERGEO_H

#include "componentchooser.h"

class ComponentChooserGeo : public ComponentChooser
{
public:
    explicit ComponentChooserGeo(QObject *parent);
    void save() override;
};

#endif
