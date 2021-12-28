/*
    SPDX-FileCopyrightText: 2021 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserpdfviewer.h"

ComponentChooserWordProcessor::ComponentChooserWordProcessor(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("application/pdf"),
                       QStringLiteral("WordProcessor"),
                       QStringLiteral("libreoffice-writer.desktop"),
                       i18n("Select default word processor"))
{
}

void ComponentChooserWordProcessor::save()
{
    saveMimeTypeAssociation(QStringLiteral("application/pdf"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
