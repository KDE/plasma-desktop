/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

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

static const QStringList textEditorMimetypes{"text/plain",
                                             "text/x-cmake",
                                             "text/markdown",
                                             "application/x-docbook+xml",
                                             "application/json",
                                             "application/x-yaml"};

QStringList ComponentChooserTextEditor::mimeTypes() const
{
    return textEditorMimetypes;
}
