/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>

class InputMethod;

class InputBracket : public QObject
{
    Q_OBJECT
    Q_PROPERTY(InputMethod *inputMethod READ inputMethod WRITE setInputMethod NOTIFY inputMethodChanged)
    Q_PROPERTY(QString bracketedText READ bracketedText NOTIFY bracketedTextChanged)
public:
    InputMethod *inputMethod() const
    {
        return m_inputMethod;
    }

    void setInputMethod(InputMethod *im);
    QString bracketedText() const
    {
        return m_bracketedText;
    }

    /// removes whatever was in the bracket (included) and replaces it with @p text
    Q_INVOKABLE void replaceBracket(const QString &text);

Q_SIGNALS:
    void inputMethodChanged(InputMethod *inputMethod);
    void bracketedTextChanged(const QString &bracketedText);

private:
    void setBracketedText(const QString &text);
    void refreshBracketedText(const QString &text, uint32_t cursor, uint32_t anchor);
    void currentChanged();

    QString m_bracketedText;
    InputMethod *m_inputMethod = nullptr;
};
