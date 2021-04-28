/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <QObject>
#include <qwayland-input-method-unstable-v1.h>

class InputMethodContext : public QObject, public QtWayland::zwp_input_method_context_v1
{
    Q_OBJECT
    Q_PROPERTY(QString surroundingText READ surroundingText NOTIFY surroundingTextChanged)
public:
    explicit InputMethodContext(struct ::zwp_input_method_context_v1 *id);
    ~InputMethodContext() override;

    enum ContentHint {
        content_hint_none = 0x0, // no special behaviour
        content_hint_default = 0x7, // auto completion, correction and capitalization
        content_hint_password = 0xc0, // hidden and sensitive text
        content_hint_auto_completion = 0x1, // suggest word completions
        content_hint_auto_correction = 0x2, // suggest word corrections
        content_hint_auto_capitalization = 0x4, // switch to uppercase letters at the start of a sentence
        content_hint_lowercase = 0x8, // prefer lowercase letters
        content_hint_uppercase = 0x10, // prefer uppercase letters
        content_hint_titlecase = 0x20, // prefer casing for titles and headings (can be language dependent)
        content_hint_hidden_text = 0x40, // characters should be hidden
        content_hint_sensitive_data = 0x80, // typed text should not be stored
        content_hint_latin = 0x100, // just latin characters should be entered
        content_hint_multiline = 0x200, // the text input is multiline
    };
    Q_ENUM(ContentHint)

    enum ContentPurpose {
        content_purpose_normal = 0, // default input, allowing all characters
        content_purpose_alpha = 1, // allow only alphabetic characters
        content_purpose_digits = 2, // allow only digits
        content_purpose_number = 3, // input a number (including decimal separator and sign)
        content_purpose_phone = 4, // input a phone number
        content_purpose_url = 5, // input an URL
        content_purpose_email = 6, // input an email address
        content_purpose_name = 7, // input a name of a person
        content_purpose_password = 8, // input a password (combine with password or sensitive_data hint)
        content_purpose_date = 9, // input a date
        content_purpose_time = 10, // input a time
        content_purpose_datetime = 11, // input a date and time
        content_purpose_terminal = 12, // input for a terminal
    };
    Q_ENUM(ContentPurpose)

    Q_INVOKABLE void commitString(const QString &string);

    QString surroundingText() const
    {
        return m_text;
    }
    uint32_t cursor() const
    {
        return m_cursor;
    }
    uint32_t anchor() const
    {
        return m_anchor;
    }

Q_SIGNALS:
    void reset();
    void commit(uint32_t serial);
    void contentType(ContentHint hint, ContentPurpose purpose);
    void preferredLanguage(const QString &language);
    void surroundingTextChanged(const QString &surroundingText, uint32_t cursor, uint32_t anchor);

private:
    void zwp_input_method_context_v1_surrounding_text(const QString &text, uint32_t cursor, uint32_t anchor) override;
    void zwp_input_method_context_v1_reset() override;
    void zwp_input_method_context_v1_content_type(uint32_t hint, uint32_t purpose) override;
    void zwp_input_method_context_v1_invoke_action(uint32_t button, uint32_t index) override;
    void zwp_input_method_context_v1_commit_state(uint32_t serial) override;
    void zwp_input_method_context_v1_preferred_language(const QString &language) override;

    QString m_text;
    uint32_t m_cursor = 0;
    uint32_t m_anchor = 0;
    uint32_t m_latestSerial = 0;
};

class InputMethod : public QObject, public QtWayland::zwp_input_method_v1
{
    Q_OBJECT
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    Q_PROPERTY(InputMethodContext *current READ current NOTIFY activeChanged)
public:
    explicit InputMethod(struct ::wl_registry *registry, int id, int version);
    ~InputMethod() override;

    void zwp_input_method_v1_activate(struct ::zwp_input_method_context_v1 *id) override;
    void zwp_input_method_v1_deactivate(struct ::zwp_input_method_context_v1 *context) override;

    InputMethodContext *current() const
    {
        return m_current;
    }

    bool isActive() const
    {
        return bool(m_current);
    }

Q_SIGNALS:
    void activate();
    void deactivate();
    void activeChanged(bool active);

private:
    quint32 m_serial = 0;
    InputMethodContext *m_current = nullptr;
};
