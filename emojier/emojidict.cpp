/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "emojidict.h"
#include "emojicategory.h"
#include "emojier_debug.h"
#include <KLocalizedString>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>

QString Emoji::categoryName() const
{
    const auto &names = getCategoryNames();
    if (category <= 0 || category > names.size()) {
        return QString();
    }
    // off by 1 because 0 is unknown.
    return names[category - 1];
}

void replaceEmoji(Emoji &currentEmoji, const Emoji &newEmoji)
{
    const QString fallbackDescription = currentEmoji.description;
    const int skinTone = currentEmoji.skinTone;
    const int skinToneVariantIndex = currentEmoji.skinToneVariantIndex;
    const int twoToneVariantIndex = currentEmoji.twoToneVariantIndex;

    currentEmoji = newEmoji;

    currentEmoji.fallbackDescription = fallbackDescription;
    currentEmoji.skinTone = skinTone;
    currentEmoji.skinToneVariantIndex = skinToneVariantIndex;
    currentEmoji.twoToneVariantIndex = twoToneVariantIndex;
}

void EmojiDict::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    auto buffer = file.readAll();
    buffer = qUncompress(buffer);
    QDataStream stream(&buffer, QIODevice::ReadOnly);
    // We use a fixed version to keep it binary compatible.
    // Also we do not use advanced data type so it does not matter.
    stream.setVersion(QDataStream::Qt_5_15);
    // Explicitly set endianness to ensure it's not relevant to architecture.
    stream.setByteOrder(QDataStream::LittleEndian);
    QList<Emoji> emojis;
    stream >> emojis;

    int skinToneVariantIndex = 0;
    int twoToneVariantIndex = 0;
    bool searchNeutralForTwoTone = false;
    for (const auto &emoji : emojis) {
        if (auto iter = m_processedEmojis.find(emoji.content); iter != m_processedEmojis.end()) {
            // Overwrite with new data but keep previous description as fallback.
            replaceEmoji(m_emojis[iter.value()], emoji);
        } else if (auto iter = m_processedTonedEmojis.find(emoji.content); iter != m_processedTonedEmojis.end()) {
            replaceEmoji(m_tonedEmojis[iter.value()], emoji);
        } else if (auto iter = m_processedTwoToneEmojis.find(emoji.content); iter != m_processedTwoToneEmojis.end()) {
            replaceEmoji(m_twoToneEmojis[iter.value()], emoji);
        } else {
            if (emoji.skinTone == Tone::TwoTone) {
                if (searchNeutralForTwoTone && !m_emojis.isEmpty()) {
                    twoToneVariantIndex += TWO_TONE_PAIR_PERMUTATIONS;
                    auto &neutralEmoji = m_emojis.last();
                    neutralEmoji.twoToneVariantIndex = twoToneVariantIndex;
                }
                m_processedTwoToneEmojis[emoji.content] = m_twoToneEmojis.size();
                m_twoToneEmojis.append(emoji);
                m_twoToneEmojis.last().skinToneVariantIndex = skinToneVariantIndex - SKIN_TONE_COUNT;
                searchNeutralForTwoTone = false;
            } else if (emoji.skinTone > Tone::Neutral) {
                if (emoji.skinTone == Tone::Light && !m_emojis.isEmpty()) {
                    auto &neutralEmoji = m_emojis.last();
                    neutralEmoji.skinTone = Tone::Neutral;
                    neutralEmoji.skinToneVariantIndex = skinToneVariantIndex;
                    skinToneVariantIndex += SKIN_TONE_COUNT;
                }
                m_processedTonedEmojis[emoji.content] = m_tonedEmojis.size();
                m_tonedEmojis.append(emoji);
            } else {
                m_processedEmojis[emoji.content] = m_emojis.size();
                m_emojis.append(emoji);
                searchNeutralForTwoTone = true;
            }
        }
    }

    if (skinToneVariantIndex != 0 && m_tonedEmojis.size() != skinToneVariantIndex) {
        qCWarning(EMOJIER_LOG) << "Some skin tone variants are missing.";
        m_emojis.append(std::move(m_tonedEmojis));
        m_emojis.append(std::move(m_twoToneEmojis));
        for (auto &emoji : m_emojis) {
            emoji.skinTone = Tone::HasNoVariants;
            emoji.twoToneVariantIndex = 0;
        }
    } else if (twoToneVariantIndex != 0 && m_twoToneEmojis.size() != twoToneVariantIndex) {
        qCWarning(EMOJIER_LOG) << "Some two-tone skin variants are missing.";
        m_emojis.append(std::move(m_twoToneEmojis));
        for (auto &emoji : m_emojis) {
            emoji.twoToneVariantIndex = 0;
        }
    }
}
