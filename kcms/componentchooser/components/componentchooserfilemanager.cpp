/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

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

static const QStringList fileManagerMimetypes{"inode/directory"};

QStringList ComponentChooserFileManager::mimeTypes() const
{
    return fileManagerMimetypes;
}
