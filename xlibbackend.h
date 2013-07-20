#ifndef XLIBBACKEND_H
#define XLIBBACKEND_H

#include <QMap>
#include <QSet>
#include <QSharedPointer>
#include <QLatin1String>

#include "touchpadbackend.h"

#include <X11/Xlib.h>
#include <xcb/xcb.h>
#include <X11/extensions/XInput.h>
#include <xcb/xinput.h>

#include "xcbatom.h"

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    XlibBackend(QObject *parent = 0);
    ~XlibBackend();

    void applyConfig(const TouchpadParameters *);
    void getConfig(TouchpadParameters *);
    QStringList supportedParameters() { init(); return m_supported; }

private:
    bool init();
    bool m_triedInit;

    struct PropertyInfo *getDevProperty(const QLatin1String &propName);

    bool setParameter(const QString &name, const QVariant &);
    bool getParameter(const QString &name, QVariant &);

    QSharedPointer<Display> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_floatType, m_touchpadType, m_capsAtom;
    xcb_input_list_input_devices_cookie_t m_devicesCookie;

    QSharedPointer<XDevice> findTouchpad();
    QSharedPointer<XDevice> m_device;

    QMap<QLatin1String, QSharedPointer<XcbAtom> > m_atoms;
    QMap<QLatin1String, struct PropertyInfo> m_props;
    QSet<QLatin1String> m_changed;
    QStringList m_supported;
};

#endif // XLIBBACKEND_H
