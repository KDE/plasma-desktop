/*
 *  Copyright (C) 2014 Weng Xuetian <wengxt@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "enginemanager.h"

#define THRESHOLD 256

EngineManager::EngineManager() : m_engines(0), m_length(0)
{

}

EngineManager::~EngineManager()
{
    freeOldEngine();
}

void EngineManager::freeOldEngine()
{
    if (m_engines) {
        int i = 0;
        while (i < length()) {
            g_object_unref(m_engines[i]);
            i++;
        }
        g_free(m_engines);
    }
}

void EngineManager::setEngines(IBusEngineDesc** engines, int length)
{
    freeOldEngine();
    m_engineMap.clear();
    m_history.clear();

    m_engines = engines;
    m_length = length;
}

void EngineManager::setUseGlobalEngine(gboolean use)
{
    m_useGlobalEngine = use;
    if (use) {
        m_engineMap.clear();
    }
}

void EngineManager::setCurrentContext(const gchar* input_context_path)
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

void EngineManager::setCurrentEngine(const char* name)
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

const char* EngineManager::navigate(IBusEngineDesc* engine)
{
    if (length() == 0) {
        return "";
    }

    int i = 0;
    while (i < length()) {
        if (m_engines[i] == engine ||
            0 == g_strcmp0(ibus_engine_desc_get_name(engine), ibus_engine_desc_get_name(m_engines[i]))) {
            break;
        }
        i++;
    }
    i = (i + 1) % length();
    return ibus_engine_desc_get_name(m_engines[i]);
}
