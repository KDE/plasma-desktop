/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooserarchivemanager.h"

ComponentChooserArchiveManager::ComponentChooserArchiveManager(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("application/zip"),
                       QStringLiteral("Archiving"),
                       QStringLiteral("org.kde.ark.desktop"),
                       i18n("Select default archive manager"))
{
}


void ComponentChooserArchiveManager::save()
{
    QStringList mimetypes{"application/x-tar",
                          "application/x-compressed-tar",
                          "application/x-bzip-compressed-tar",
                          "application/x-tarz",
                          "application/x-xz-compressed-tar",
                          "application/x-lzma-compressed-tar",
                          "application/x-lzip-compressed-tar",
                          "application/x-tzo",
                          "application/x-lrzip-compressed-tar",
                          "application/x-lz4-compressed-tar",
                          "application/x-zstd-compressed-tar",
                          "application/x-cd-image",
                          "application/x-bcpio",
                          "application/x-cpio",
                          "application/x-cpio-compressed",
                          "application/x-sv4cpio",
                          "application/x-sv4crc",
                          "application/x-source-rpm",
                          "application/vnd.ms-cab-compressed",
                          "application/x-xar",
                          "application/x-iso9660-appimage",
                          "application/x-archive",
                          "application/vnd.rar",
                          "application/x-rar",
                          "application/x-7z-compressed",
                          "application/zip",
                          "application/x-compress",
                          "application/gzip",
                          "application/x-bzip",
                          "application/x-lzma",
                          "application/x-xz",
                          "application/zstd",
                          "application/x-lha"};
    saveMimeTypeAssociations(mimetypes, m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
