/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserpdfviewer.h"

ComponentChooserPdfViewer::ComponentChooserPdfViewer(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("application/pdf"),
                       QStringLiteral("Viewer"),
                       QStringLiteral("okularApplication_pdf.desktop"),
                       i18n("Select default PDF viewer"))
{
}

QStringList pdfMimetypes{"application/pdf"};

void ComponentChooserPdfViewer::save()
{
    saveMimeTypeAssociations(pdfMimetypes, m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
