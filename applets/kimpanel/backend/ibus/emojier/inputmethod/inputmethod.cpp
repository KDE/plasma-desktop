/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "inputmethod.h"
#include <QDateTime>
#include <QDebug>
#include <QGuiApplication>
#include <QInputMethod>
#include <QStandardPaths>

InputMethod::InputMethod(struct ::wl_registry *registry, int id, int version)
    : QtWayland::zwp_input_method_v1(registry, id, version)
{
}

InputMethod::~InputMethod() = default;

void InputMethod::zwp_input_method_v1_activate(struct ::zwp_input_method_context_v1 *id)
{
    m_current = new InputMethodContext(id);
    Q_EMIT activeChanged(true);
    Q_EMIT activate();
}

void InputMethod::zwp_input_method_v1_deactivate(struct ::zwp_input_method_context_v1 *context)
{
    Q_EMIT activeChanged(false);
    Q_EMIT deactivate();
    Q_ASSERT(m_current->object() == context);
    delete m_current;
    m_current = nullptr;
}

InputMethodContext::InputMethodContext(struct ::zwp_input_method_context_v1 *id)
    : QtWayland::zwp_input_method_context_v1(id)
{
}

InputMethodContext::~InputMethodContext() = default;

void InputMethodContext::zwp_input_method_context_v1_reset()
{
    Q_EMIT reset();
}

void InputMethodContext::zwp_input_method_context_v1_commit_state(uint32_t serial)
{
    m_latestSerial = serial;
    Q_EMIT commit(serial);
}

void InputMethodContext::zwp_input_method_context_v1_content_type(uint32_t hint, uint32_t purpose)
{
    Q_EMIT contentType(ContentHint(hint), ContentPurpose(purpose));
}

void InputMethodContext::zwp_input_method_context_v1_preferred_language(const QString &language)
{
    Q_EMIT preferredLanguage(language);
}

void InputMethodContext::zwp_input_method_context_v1_surrounding_text(const QString &text, uint32_t cursor, uint32_t anchor)
{
    m_text = text;
    m_cursor = cursor;
    m_anchor = anchor;
    Q_EMIT surroundingTextChanged(text, cursor, anchor);
}

void InputMethodContext::zwp_input_method_context_v1_invoke_action(uint32_t button, uint32_t index)
{
}

void InputMethodContext::commitString(const QString &string)
{
    commit_string(m_latestSerial, string);
}
