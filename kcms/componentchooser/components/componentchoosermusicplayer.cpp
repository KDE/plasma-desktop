/*
    SPDX-FileCopyrightText: 2022 Thiago Sueto <herzenschein@gmail.com>
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

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

static const QStringList audioMimetypes{"audio/aac",
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

QStringList ComponentChooserMusicPlayer::mimeTypes() const
{
    return audioMimetypes;
}
