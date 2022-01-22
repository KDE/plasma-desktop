/*
    SPDX-FileCopyrightText: 2021 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchoosermatrix.h"

ComponentChooserMatrix::ComponentChooserMatrix(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("x-scheme-handler/matrix"),
                       QStringLiteral("InstantMessaging"),
                       QStringLiteral("org.kde.neochat.desktop"),
                       i18n("Select default Matrix client"))
{
}

void ComponentChooserMatrix::save()
{
    saveMimeTypeAssociation(QStringLiteral("x-scheme-handler/matrix"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
