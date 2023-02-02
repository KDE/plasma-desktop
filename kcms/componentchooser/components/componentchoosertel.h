/*
    SPDX-FileCopyrightText: 2022 Kai Uwe Broulik <kde@broulik.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "componentchooser.h"

class ComponentChooserTel : public ComponentChooser
{
public:
    explicit ComponentChooserTel(QObject *parent);

    QStringList mimeTypes() const override;
};
