/*
 *    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
 *    SPDX-FileCopyrightText: 2020 Méven Car <meven.car@kdemail.net>
 *    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "componentchooserbrowser.h"

#include "browser_settings.h"

ComponentChooserBrowser::ComponentChooserBrowser(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("x-scheme-handler/http"),
                       QStringLiteral("WebBrowser"),
                       QStringLiteral("org.kde.falkon.desktop"),
                       i18n("Select default browser"))
{
}

void ComponentChooserBrowser::save()
{
    QStringList mimetypes{"x-scheme-handler/http", "x-scheme-handler/https"};
    const QString storageId = m_applications[m_index].toMap()["storageId"].toString();

    BrowserSettings browserSettings;
    browserSettings.setBrowserApplication(storageId);
    browserSettings.save();

    saveMimeTypeAssociations(mimetypes, storageId);
}
