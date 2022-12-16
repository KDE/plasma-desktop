/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include <QByteArray>
#include <QDataStream>
#include <QJsonDocument>
#include <QList>

struct Emoji {
    QString content;
    QString description;
    qint32 category;
    QStringList annotations;

    QString categoryName() const;
};

inline QDataStream &operator>>(QDataStream &stream, Emoji &emoji)
{
    QByteArray buffer;
    stream >> buffer;
    emoji.content = QString::fromUtf8(buffer);
    stream >> buffer;
    emoji.description = QString::fromUtf8(buffer);
    stream >> emoji.category;
    QList<QByteArray> annotationBuffers;
    stream >> annotationBuffers;
    for (const auto &annotation : annotationBuffers) {
        emoji.annotations << QString::fromUtf8(annotation);
    }
    return stream;
}

struct EmojiDict {
    void load(const QString &path);

    QList<Emoji> m_emojis;
    QMap<QString, int> m_processedEmojis;
};
