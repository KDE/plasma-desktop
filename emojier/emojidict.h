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

constexpr int SKIN_TONE_COUNT = 5;
constexpr int TWO_TONE_PAIR_PERMUTATIONS = SKIN_TONE_COUNT * (SKIN_TONE_COUNT - 1);

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
    int twoToneVariantIndex = 0;

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

    int toneLightModifierCount = 0, toneMediumLightModifierCount = 0, toneMediumModifierCount = 0, toneMediumDarkModifierCount = 0, toneDarkModifierCount = 0;
    for (const auto &c : emoji.content.toUcs4()) {
        switch (c) {
        case CharLight:
            toneLightModifierCount++;
            break;
        case CharMediumLight:
            toneMediumLightModifierCount++;
            break;
        case CharMedium:
            toneMediumModifierCount++;
            break;
        case CharMediumDark:
            toneMediumDarkModifierCount++;
            break;
        case CharDark:
            toneDarkModifierCount++;
            break;
        }
    }
    const int toneModifierCount =
        toneLightModifierCount + toneMediumLightModifierCount + toneMediumModifierCount + toneMediumDarkModifierCount + toneDarkModifierCount;

    if (toneModifierCount == 0) {
        emoji.skinTone = Tone::HasNoVariants;
    } else {
        if (toneModifierCount == toneLightModifierCount) {
            emoji.skinTone = Tone::Light;
        } else if (toneModifierCount == toneMediumLightModifierCount) {
            emoji.skinTone = Tone::MediumLight;
        } else if (toneModifierCount == toneMediumModifierCount) {
            emoji.skinTone = Tone::Medium;
        } else if (toneModifierCount == toneMediumDarkModifierCount) {
            emoji.skinTone = Tone::MediumDark;
        } else if (toneModifierCount == toneDarkModifierCount) {
            emoji.skinTone = Tone::Dark;
        } else if (toneModifierCount == 2) {
            emoji.skinTone = Tone::TwoTone;
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
    QList<Emoji> m_twoToneEmojis;

    QMap<QString, int> m_processedEmojis;
    QMap<QString, int> m_processedTonedEmojis;
    QMap<QString, int> m_processedTwoToneEmojis;
};
