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

void ComponentChooserMusicPlayer::save()
{
    saveMimeTypeAssociation(QStringLiteral("application/x-ogm-audio"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/aac"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/mp4"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/mpeg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/mpegurl"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/ogg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/vnd.rn-realaudio"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/vorbis"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-flac"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-mp3"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-mpegurl"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-ms-wma"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-musepack"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-oggflac"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-pn-realaudio"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-scpls"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-speex"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-vorbis"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-vorbis+ogg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("audio/x-wav"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
