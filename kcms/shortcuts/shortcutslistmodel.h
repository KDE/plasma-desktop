/*
    SPDX-FileCopyrightText: 2026 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KDescendantsProxyModel>
#include <kdescendantsproxymodel.h>
#include <qtmetamacros.h>

#include "kglobalaccelmodel_export.h"

class KGLOBALACCELMODEL_EXPORT ShortcutsListModel : public KDescendantsProxyModel
{
    Q_OBJECT

public:
    explicit ShortcutsListModel(QObject *parent);
};
