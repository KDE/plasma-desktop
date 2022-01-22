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
    saveMimeTypeAssociation(QStringLiteral("video/mp4"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/mpeg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/msvideo"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/ogg"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/avi"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/flv"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/mkv"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/webm"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
    saveMimeTypeAssociation(QStringLiteral("video/3gp"), m_applications[m_index].toMap()[QStringLiteral("storageId")].toString());
}
