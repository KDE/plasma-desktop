/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "emojiersettings.h"
#include <QByteArray>
#include <QDataStream>
#include <QJsonDocument>
#include <QList>
#include <QMap>

using Tone = EmojierSettings::SkinTone;

enum ToneChars : uint {
    CharLight = 0x1F3FB,
    CharMediumLight = 0x1F3FC,
    CharMedium = 0x1F3FD,
    CharMediumDark = 0x1F3FE,
    CharDark = 0x1F3FF,
};

struct Emoji {
    QString content;
    QString description;
    QString fallbackDescription;
    qint32 category;
    QStringList annotations;
    int skinTone;
    int skinToneVariantIndex;

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

    int sumToneL = 0, sumToneML = 0, sumToneM = 0, sumToneMD = 0, sumToneD = 0;
    for (const auto &c : emoji.content.toUcs4()) {
        switch (c) {
        case CharLight:
            sumToneL++;
            break;
        case CharMediumLight:
            sumToneML++;
            break;
        case CharMedium:
            sumToneM++;
            break;
        case CharMediumDark:
            sumToneMD++;
            break;
        case CharDark:
            sumToneD++;
            break;
        }
    }
    const int sumToneAll = sumToneL + sumToneML + sumToneM + sumToneMD + sumToneD;

    if (sumToneAll == 0) {
        emoji.skinTone = Tone::HasNoVariants;
    } else {
        if (sumToneL == sumToneAll) {
            emoji.skinTone = Tone::Light;
        } else if (sumToneML == sumToneAll || sumToneM == sumToneAll || sumToneMD == sumToneAll || sumToneD == sumToneAll) {
            emoji.skinTone = Tone::Dark;
        } else {
            emoji.skinTone = Tone::HasNoVariants;
        }
    }

    return stream;
}

struct EmojiDict {
    void load(const QString &path);

    QList<Emoji> m_emojis;
    QList<Emoji> m_tonedEmojis;
    QMap<QString, int> m_processedEmojis;
    QMap<QString, int> m_processedTonedEmojis;
};
