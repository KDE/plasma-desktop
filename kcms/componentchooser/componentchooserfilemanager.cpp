/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserfilemanager.h"

ComponentChooserFileManager::ComponentChooserFileManager(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("inode/directory"),
                       QStringLiteral("FileManager"),
                       QStringLiteral("org.kde.dolphin.desktop"),
                       i18n("Select default file manager"))
{
}

QStringList fileManagerMimetypes{"inode/directory"};

void ComponentChooserFileManager::save()
{
    saveMimeTypeAssociations(fileManagerMimetypes, m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
