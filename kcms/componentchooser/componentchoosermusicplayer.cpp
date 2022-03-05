/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchoosermusicplayer.h"

ComponentChooserMusicPlayer::ComponentChooserMusicPlayer(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("audio/mpeg"),
                       QStringLiteral("Player"),
                       QStringLiteral("org.kde.elisa.desktop"),
                       i18n("Select default music player"))
{
}

QStringList audioMimetypes{"application/x-ogm-audio",
                           "audio/aac",
                           "audio/mp4",
                           "audio/mpeg",
                           "audio/mpegurl",
                           "audio/ogg",
                           "audio/vnd.rn-realaudio",
                           "audio/vorbis",
                           "audio/x-flac",
                           "audio/x-mp3",
                           "audio/x-mpegurl",
                           "audio/x-ms-wma",
                           "audio/x-musepack",
                           "audio/x-oggflac",
                           "audio/x-pn-realaudio",
                           "audio/x-scpls",
                           "audio/x-speex",
                           "audio/x-vorbis",
                           "audio/x-vorbis+ogg",
                           "audio/x-wav"};

void ComponentChooserMusicPlayer::save()
{
    saveMimeTypeAssociations(audioMimetypes, m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
