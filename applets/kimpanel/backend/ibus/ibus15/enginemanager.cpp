/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#include "enginemanager.h"

#define THRESHOLD 256

EngineManager::EngineManager()
    : m_engines(nullptr)
    , m_length(0)
{
}

EngineManager::~EngineManager()
{
    freeOldEngine();
}

void EngineManager::freeOldEngine()
{
    if (m_engines) {
        for (size_t i = 0; i < m_length; i++) {
            g_object_unref(m_engines[i]);
        }
        g_free(m_engines);
    }
}

void EngineManager::setEngines(IBusEngineDesc **engines)
{
    freeOldEngine();
    m_engineMap.clear();
    m_history.clear();

    m_engines = engines;
    m_length = 0;
    if (engines) {
        while (engines[m_length]) {
            m_length++;
        }
    }
}

void EngineManager::setUseGlobalEngine(gboolean use)
{
    m_useGlobalEngine = use;
    if (use) {
        m_engineMap.clear();
    }
}

void EngineManager::setCurrentContext(const gchar *input_context_path)
{
    m_currentContext = QString::fromLatin1(input_context_path);
}

QString EngineManager::currentEngine()
{
    if (m_engineMap.contains(m_currentContext)) {
        return m_engineMap[m_currentContext];
    } else if (m_length > 0) {
        return QString::fromUtf8(ibus_engine_desc_get_name(m_engines[0]));
    } else {
        return QString();
    }
}

void EngineManager::setCurrentEngine(const char *name)
{
    if (!m_useGlobalEngine && !m_currentContext.isEmpty()) {
        if (!m_engineMap.contains(m_currentContext)) {
            m_history.enqueue(m_currentContext);
        }
        m_engineMap[m_currentContext] = QString::fromUtf8(name);
        if (m_engineMap.size() > THRESHOLD) {
            m_engineMap.remove(m_history.dequeue());
            Q_ASSERT(m_engineMap.size() == m_history.size());
        }
    }
}

const char *EngineManager::navigate(IBusEngineDesc *engine, bool forward)
{
    if (length() == 0) {
        return "";
    }

    size_t i = 0;
    for (; i < m_length; i++) {
        if (m_engines[i] == engine || 0 == g_strcmp0(ibus_engine_desc_get_name(engine), ibus_engine_desc_get_name(m_engines[i]))) {
            break;
        }
    }
    i = (i + (forward ? 1 : (length() - 1))) % length();
    return ibus_engine_desc_get_name(m_engines[i]);
}

void EngineManager::moveToFirst(IBusEngineDesc *engine)
{
    size_t i = 0;
    while (i < length()) {
        if (m_engines[i] == engine || 0 == g_strcmp0(ibus_engine_desc_get_name(engine), ibus_engine_desc_get_name(m_engines[i]))) {
            break;
        }
        i++;
    }
    if (i == 0) {
        return;
    }
    if (i >= m_length) {
        return;
    }

    engine = m_engines[i];

    for (int j = i; j > 0; j--) {
        m_engines[j] = m_engines[j - 1];
    }
    m_engines[0] = engine;
}

size_t EngineManager::getIndexByName(const char *name)
{
    size_t i = 0;
    for (; i < m_length; i++) {
        if (0 == g_strcmp0(name, ibus_engine_desc_get_name(m_engines[i]))) {
            break;
        }
    }
    return i;
}

void EngineManager::setOrder(const gchar **engine_names, size_t len)
{
    size_t k = 0;
    for (size_t i = 0; i < len; i++) {
        size_t j = getIndexByName(engine_names[i]);
        if (j < length() && j >= k) {
            if (j != k) {
                IBusEngineDesc *temp = m_engines[k];
                m_engines[k] = m_engines[j];
                m_engines[j] = temp;
            }
            k++;
        }
    }
}

QStringList EngineManager::engineOrder()
{
    QStringList list;
    for (size_t i = 0; i < m_length; i++) {
        list << QString::fromUtf8(ibus_engine_desc_get_name(m_engines[i]));
    }
    return list;
}
