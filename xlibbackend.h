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

#include "xcbatom.h"

enum TouchpadCapabilitiy
{
    TouchpadHasLeftButton,
    TouchpadHasMiddleButton,
    TouchpadHasRightButton,
    TouchpadTwoFingerDetect,
    TouchpadThreeFingerDetect,
    TouchpadPressureDetect,
    TouchpadPalmDetect,
    TouchpadCapsCount
};

class XlibBackend : public TouchpadBackend
{
    Q_OBJECT

public:
    XlibBackend(QObject *parent = 0);
    ~XlibBackend();

    void applyConfig(const TouchpadParameters *);
    void getConfig(TouchpadParameters *, QStringList *supportedParameters = 0);
    bool test();

private:
    struct PropertyInfo *getDevProperty(const QLatin1String &propName);

    bool setParameter(const QString &name, const QVariant &);
    bool getParameter(const QString &name, QVariant &);

    QSharedPointer<Display> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_floatType;

    QSharedPointer<XDevice> m_device;

    QMap<QLatin1String, QSharedPointer<XcbAtom> > m_atoms;
    QMap<QLatin1String, struct PropertyInfo> m_props;
    QSet<QLatin1String> m_changed;

    bool m_caps[TouchpadCapsCount];
};

#endif // XLIBBACKEND_H
