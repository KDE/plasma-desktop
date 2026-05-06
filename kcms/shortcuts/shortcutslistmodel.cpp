/*
    SPDX-FileCopyrightText: 2026 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "shortcutslistmodel.h"

#include <kdescendantsproxymodel.h>

ShortcutsListModel::ShortcutsListModel(QObject *parent)
    : KDescendantsProxyModel(parent)
{
}

#include "moc_shortcutslistmodel.cpp"
