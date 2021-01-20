/***************************************************************************
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

#include "componentchooserfilemanager.h"

ComponentChooserFileManager::ComponentChooserFileManager(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("inode/directory"),
                       QStringLiteral("FileManager"),
                       QStringLiteral("org.kde.dolphin.desktop"),
                       i18n("Select default file manager"))
{
}

void ComponentChooserFileManager::save()
{
    saveMimeTypeAssociation(QStringLiteral("inode/directory"), m_applications[m_index].toMap()["storageId"].toString());
}
