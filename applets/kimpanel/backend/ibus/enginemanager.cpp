#include "enginemanager.h"

EngineManager::EngineManager() : m_engines(0)
{

}

EngineManager::~EngineManager()
{
    if (m_engines) {
    }
}

void EngineManager::setEngines(IBusEngineDesc** engines)
{
    if (m_engines) {
        int i = 0;
        while (m_engines[i]) {
            g_object_unref(m_engines);
            i++;
        }
        g_free(m_engines);
    }

    m_engines = engines;
}
