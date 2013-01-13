#ifndef PROPERTYMANAGER_H
#define PROPERTYMANAGER_H
#include <ibus.h>
#include <QByteArray>

class PropertyManager {
public:
    PropertyManager();
    virtual ~PropertyManager();

    void setProperties(IBusPropList* props);
    void updateProperty(IBusProperty* prop);
    IBusProperty* property(const QByteArray& key);
    IBusPropList* properties() { return m_props; }

private:
    IBusPropList* m_props;
    IBusProperty* searchList(const QByteArray& key, IBusPropList* props);
};

#endif // PROPERTYMANAGER_H
