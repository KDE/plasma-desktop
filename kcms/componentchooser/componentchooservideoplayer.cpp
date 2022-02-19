/*
     SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

     SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchooservideoplayer.h"

ComponentChooserVideoPlayer::ComponentChooserVideoPlayer(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("video/mp4"),
                       QStringLiteral("Video"),
                       QStringLiteral("org.kde.haruna.desktop"),
                       i18n("Select default video player"))
{
}

void ComponentChooserVideoPlayer::save()
{
    QStringList mimetypes{"video/3gp",
                          "video/3gpp",
                          "video/3gpp2",
                          "video/avi",
                          "video/divx",
                          "video/dv",
                          "video/fli",
                          "video/flv",
                          "video/mp2t",
                          "video/mp4",
                          "video/mp4v-es",
                          "video/mpeg",
                          "video/msvideo",
                          "video/ogg",
                          "video/quicktime",
                          "video/vnd.divx",
                          "video/vnd.mpegurl",
                          "video/vnd.rn-realvideo",
                          "video/webm",
                          "video/x-avi",
                          "video/x-flc",
                          "video/x-flv",
                          "video/x-m4v",
                          "video/x-matroska",
                          "video/x-mpeg2",
                          "video/x-ms-asf",
                          "video/x-msvideo",
                          "video/x-ms-wmv",
                          "video/x-ms-wmx",
                          "video/x-ogm",
                          "video/x-ogm+ogg",
                          "video/x-theora",
                          "video/x-theora+ogg",
                          "application/mxf",
                          "application/ogg",
                          "application/sdp",
                          "application/vnd.apple.mpegurl",
                          "application/vnd.ms-asf",
                          "application/vnd.rn-realmedia",
                          "application/vnd.rn-realmedia-vbr",
                          "application/x-extension-m4a",
                          "application/x-extension-mp4",
                          "application/x-matroska",
                          "application/x-ogg"};
    saveMimeTypeAssociations(mimetypes, m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
