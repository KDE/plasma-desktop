/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "emojidict.h"
#include "emojicategory.h"
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
    for (const auto &emoji : emojis) {
        if (auto iter = m_processedEmojis.find(emoji.content); iter != m_processedEmojis.end()) {
            // Overwrite with new data but keep previous description as fallback.
            auto &foundEmoji = m_emojis[iter.value()];
            const QString fallbackDescription = foundEmoji.description;
            const int skinTone = foundEmoji.skinTone;
            const int oldSkinToneVariantIndex = foundEmoji.skinToneVariantIndex;
            foundEmoji = emoji;
            foundEmoji.fallbackDescription = fallbackDescription;
            foundEmoji.skinTone = skinTone;
            foundEmoji.skinToneVariantIndex = oldSkinToneVariantIndex;
        } else if (auto iter = m_processedTonedEmojis.find(emoji.content); iter != m_processedTonedEmojis.end()) {
            // Overwrite with new data but keep previous description as fallback.
            auto &foundEmoji = m_tonedEmojis[iter.value()];
            const QString fallbackDescription = foundEmoji.description;
            foundEmoji = emoji;
            foundEmoji.fallbackDescription = fallbackDescription;
        } else {
            if (emoji.skinTone > Tone::Neutral) {
                if (emoji.skinTone == Tone::Light && !m_emojis.isEmpty()) {
                    auto &neutralEmoji = m_emojis[m_emojis.size() - 1];
                    neutralEmoji.skinTone = Tone::Neutral;
                    neutralEmoji.skinToneVariantIndex = skinToneVariantIndex;
                    skinToneVariantIndex += 5;
                }
                m_processedTonedEmojis[emoji.content] = m_tonedEmojis.size();
                m_tonedEmojis.append(emoji);
            } else {
                m_processedEmojis[emoji.content] = m_emojis.size();
                m_emojis.append(emoji);
            }
        }
    }

    if (skinToneVariantIndex != 0 && m_tonedEmojis.size() != skinToneVariantIndex) {
        qWarning() << "Some skin tone variants are missing.";
        m_emojis.append(std::move(m_tonedEmojis));
        for (auto &emoji : m_emojis) {
            emoji.skinTone = Tone::HasNoVariants;
        }
    }
}
