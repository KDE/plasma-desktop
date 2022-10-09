/*
    SPDX-FileCopyrightText: 2022 Weng Xuetian <wegnxt@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
// Generated from emoji-test.txt
#include "emojicategory.h"

#include <KLazyLocalizedString>

const QStringList &getCategoryNames()
{
    static const QStringList names = {
        QString::fromUtf8(kli18nc("Emoji Category", "Smileys and Emotion").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "People and Body").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Component").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Animals and Nature").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Food and Drink").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Travel and Places").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Activities").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Objects").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Symbols").untranslatedText()),
        QString::fromUtf8(kli18nc("Emoji Category", "Flags").untranslatedText()),
    };
    return names;
}
