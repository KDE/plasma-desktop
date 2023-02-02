/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserpdfviewer.h"

ComponentChooserPdfViewer::ComponentChooserPdfViewer(QObject *parent)
    : ComponentChooser(parent, QStringLiteral("application/pdf"), QString(), QStringLiteral("okularApplication_pdf.desktop"), i18n("Select default PDF viewer"))
{
}

static const QStringList pdfMimetypes{"application/pdf"};

QStringList ComponentChooserPdfViewer::mimeTypes() const
{
    return pdfMimetypes;
}
