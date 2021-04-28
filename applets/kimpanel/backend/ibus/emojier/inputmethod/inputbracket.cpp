/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "inputbracket.h"
#include "inputmethod.h"
#include <QDebug>

void InputBracket::setInputMethod(InputMethod *im)
{
    if (m_inputMethod == im) {
        return;
    }

    if (m_inputMethod) {
        disconnect(m_inputMethod, nullptr, this, nullptr);
    }
    m_inputMethod = im;
    connect(m_inputMethod, &InputMethod::activeChanged, this, &InputBracket::currentChanged);

    Q_EMIT inputMethodChanged(im);
    currentChanged();
}

void InputBracket::setBracketedText(const QString &text)
{
    if (m_bracketedText != text) {
        qDebug() << "bracketed" << text;
        m_bracketedText = text;
        Q_EMIT bracketedTextChanged(text);
    }
}

void InputBracket::refreshBracketedText(const QString &text, uint32_t cursor, uint32_t /*anchor*/)
{
    auto current = m_inputMethod->current();
    Q_ASSERT(current);

    const int index = text.leftRef(cursor).lastIndexOf(QLatin1Char(':'));

    if (index < 0) {
        setBracketedText({});
    } else {
        setBracketedText(text.mid(index + 1, cursor - index - 1));
    }
}

void InputBracket::currentChanged()
{
    auto current = m_inputMethod->current();
    if (!current) {
        setBracketedText({});
        return;
    }

    connect(current, &InputMethodContext::surroundingTextChanged, this, &InputBracket::refreshBracketedText);

    if (!current->surroundingText().isEmpty()) {
        refreshBracketedText(current->surroundingText(), current->cursor(), current->anchor());
    }
}

void InputBracket::replaceBracket(const QString &text)
{
    auto curr = m_inputMethod->current();
    const int index = curr->surroundingText().leftRef(curr->cursor()).lastIndexOf(QLatin1Char(':'));
    if (index < 0) {
        qWarning() << "no bracket, bro!" << curr->surroundingText();
        return;
    }
    curr->delete_surrounding_text(curr->cursor() - index, 0);
    curr->commitString(text);
}
