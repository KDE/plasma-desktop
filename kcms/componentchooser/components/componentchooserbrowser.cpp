/*
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2020 Méven Car <meven.car@kdemail.net>
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserbrowser.h"

ComponentChooserBrowser::ComponentChooserBrowser(QObject *parent)
    : ComponentChooser(parent, QStringLiteral("x-scheme-handler/http"), QString(), QStringLiteral("org.kde.falkon.desktop"), i18n("Select default browser"))
{
}

static const QStringList browserMimetypes{"x-scheme-handler/http", "x-scheme-handler/https"};

QStringList ComponentChooserBrowser::mimeTypes() const
{
    return browserMimetypes;
}

void ComponentChooserBrowser::save()
{
    const auto storageId = m_model->data(m_model->index(m_index, 0), ApplicationModel::StorageId).toString();
    saveMimeTypeAssociations(storageId, browserMimetypes);
}
