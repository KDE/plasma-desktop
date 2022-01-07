/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef COMPONENTCHOOSERTEL_H
#define COMPONENTCHOOSERTEL_H

#include "componentchooser.h"

class ComponentChooserTel : public ComponentChooser
{
public:
    explicit ComponentChooserTel(QObject *parent);
    void save() override;
};

#endif // COMPONENTCHOOSERTEL_H
