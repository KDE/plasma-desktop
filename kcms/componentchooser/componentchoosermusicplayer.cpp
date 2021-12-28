/*
    SPDX-FileCopyrightText: 2021 Thiago Sueto <herzenschein@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "componentchoosermusicplayer.h"

ComponentChooserMusicPlayer::ComponentChooserMusicPlayer(QObject *parent)
    : ComponentChooser(parent,
                       QStringLiteral("audio/mpeg"),
                       QStringLiteral("MusicPlayer"),
                       QStringLiteral("org.kde.elisa.desktop"),
                       i18n("Select default music player"))
{
}

void ComponentChooserMusicPlayer::save()
{
    saveMimeTypeAssociation(QStringLiteral("audio/mpeg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/aac"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/mp4"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/vorbis"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-flac"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-wav"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/ogg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
