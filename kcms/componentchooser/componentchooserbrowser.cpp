/***************************************************************************
 *   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>                  *
 *   Copyright (C) 2020 Méven Car <meven.car@kdemail.net>                  *
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

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
    const QString storageId = m_applications[m_index].toMap()["storageId"].toString();

    BrowserSettings browserSettings;
    browserSettings.setBrowserApplication(storageId);
    browserSettings.save();

    saveMimeTypeAssociation(QStringLiteral("x-scheme-handler/http"), storageId);
    saveMimeTypeAssociation(QStringLiteral("x-scheme-handler/https"), storageId);
}
