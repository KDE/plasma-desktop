#ifndef XLIBBACKEND_H
#define XLIBBACKEND_H

#include <QVariantList>
#include <QVector>
#include <QMap>
#include <QSet>
#include <QSharedPointer>
#include <QLatin1String>

#include "touchpadbackend.h"

#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <X11/extensions/XInput.h>

#include "xcbatom.h"
#include "synclientproperties.h"

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    XlibBackend(QObject *parent, const QVariantList &);
    ~XlibBackend();

    void applyConfig(const TouchpadParameters *);
    void getConfig(TouchpadParameters *, QStringList *supportedParameters = 0);
    bool test();

private:
    struct PropertyInfo *getDevProperty(const Parameter *par);

    bool setParameter(const QString &name, const QVariant &);
    bool getParameter(const QString &name, QVariant &);

    QSharedPointer<Display> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_floatType;

    QSharedPointer<XDevice> m_device;

    QMap<QLatin1String, QSharedPointer<XcbAtom> > m_atoms;
    QMap<QLatin1String, struct PropertyInfo> m_props;
    QSet<QLatin1String> m_changed;
};

#endif // XLIBBACKEND_H
