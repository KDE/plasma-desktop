/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchoosertexteditor.h"

ComponentChooserTextEditor::ComponentChooserTextEditor(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("text/plain"),
                       QStringLiteral("TextEditor"),
                       QStringLiteral("org.kde.kate.desktop"),
                       i18n("Select default text editor"))
{
}

void ComponentChooserTextEditor::save()
{
    saveMimeTypeAssociation(QStringLiteral("text/plain"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("text/x-cmake"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("text/markdown"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("application/x-docbook+xml"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("application/json"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("application/x-yaml"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
