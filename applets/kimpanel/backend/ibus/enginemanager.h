#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H
#include <ibus.h>
#include <QByteArray>
#include <QList>

class EngineManager {
public:
    EngineManager();
    virtual ~EngineManager();

    void setEngines(IBusEngineDesc** engines);
    IBusEngineDesc** engines() { return m_engines; }

private:
    IBusEngineDesc** m_engines;
};

#endif // PROPERTYMANAGER_H
