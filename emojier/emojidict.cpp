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
    // Explicitly set endianess to ensure it's not relevant to architecture.
    stream.setByteOrder(QDataStream::LittleEndian);
    QList<Emoji> emojis;
    stream >> emojis;
    for (const auto &emoji : emojis) {
        if (auto iter = m_processedEmojis.find(emoji.content); iter != m_processedEmojis.end()) {
            // Overwrite with new data.
            m_emojis[iter.value()] = emoji;
        } else {
            m_processedEmojis[emoji.content] = m_emojis.size();
            m_emojis.append(emoji);
        }
    }
}
