/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <cmath>
#include <cstring>

#include <QtAlgorithms>

#include <KLocalizedString>
#include <QDebug>

#include <config-X11.h>

// Includes are ordered this way because of #defines in Xorg's headers
#include "xlibbackend.h" // krazy:exclude=includes
#include "xlibnotifications.h" // krazy:exclude=includes
#include "xrecordkeyboardmonitor.h" // krazy:exclude=includes
#if HAVE_XORGLIBINPUT
#endif

#include <X11/Xatom.h>
#include <X11/Xlib-xcb.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <xserver-properties.h>

struct DeviceListDeleter {
    void operator()(XDeviceInfo *p)
    {
        if (p) {
            XFreeDeviceList(p);
        }
    }
};

void XlibBackend::XDisplayCleanup::operator()(Display *p)
{
    if (p) {
        XCloseDisplay(p);
    }
}

XlibBackend *XlibBackend::initialize(QObject *parent)
{
    XlibBackend *backend = new XlibBackend(parent);
    if (!backend->m_display) {
        delete backend;
        return nullptr;
    }
    return backend;
}

XlibBackend::~XlibBackend()
{
}

XlibBackend::XlibBackend(QObject *parent)
    : TouchpadBackend(parent)
    , m_display(XOpenDisplay(nullptr))
    , m_connection(nullptr)
{
    if (m_display) {
        m_connection = XGetXCBConnection(m_display.get());
    }

    if (!m_connection) {
        m_errorString = i18n("Cannot connect to X server");
        return;
    }

    m_mouseAtom.intern(m_connection, XI_MOUSE);
    m_keyboardAtom.intern(m_connection, XI_KEYBOARD);
    m_touchpadAtom.intern(m_connection, XI_TOUCHPAD);
    m_enabledAtom.intern(m_connection, XI_PROP_ENABLED);

    m_libinputIdentifierAtom.intern(m_connection, "libinput Send Events Modes Available");

    m_device.reset(findTouchpad());
    if (!m_device) {
        m_errorString = i18n("No touchpad found");
    }
}

XlibTouchpad *XlibBackend::findTouchpad()
{
    int nDevices = 0;
    std::unique_ptr<XDeviceInfo, DeviceListDeleter> deviceInfo(XListInputDevices(m_display.get(), &nDevices));

    for (XDeviceInfo *info = deviceInfo.get(); info < deviceInfo.get() + nDevices; info++) {
        // Make sure device is touchpad
        if (info->type != m_touchpadAtom.atom()) {
            continue;
        }
        int nProperties = 0;
        std::shared_ptr<Atom> properties(XIListProperties(m_display.get(), info->id, &nProperties), XDeleter);

        Atom *atom = properties.get();
        Atom *atomEnd = properties.get() + nProperties;
        for (; atom != atomEnd; atom++) {
#if HAVE_XORGLIBINPUT
            if (*atom == m_libinputIdentifierAtom.atom()) {
                setMode(TouchpadInputBackendMode::XLibinput);
                return new LibinputTouchpad(m_display.get(), info->id);
            }
#endif
        }
    }

    return nullptr;
}

bool XlibBackend::applyConfig(const QVariantHash &p)
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->applyConfig(p);
    if (!success) {
        m_errorString = i18n("Cannot apply touchpad configuration");
    }

    return success;
}

bool XlibBackend::applyConfig()
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->applyConfig();
    if (!success) {
        m_errorString = i18n("Cannot apply touchpad configuration");
    }

    return success;
}

bool XlibBackend::getConfig(QVariantHash &p)
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->getConfig(p);
    if (!success) {
        m_errorString = i18n("Cannot read touchpad configuration");
    }
    return success;
}

bool XlibBackend::getConfig()
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->getConfig();
    if (!success) {
        m_errorString = i18n("Cannot read touchpad configuration");
    }
    return success;
}

bool XlibBackend::getDefaultConfig()
{
    if (!m_device) {
        return false;
    }

    bool success = m_device->getDefaultConfig();
    if (!success) {
        m_errorString = i18n("Cannot read default touchpad configuration");
    }
    return success;
}

bool XlibBackend::isChangedConfig() const
{
    if (!m_device) {
        return false;
    }

    return m_device->isChangedConfig();
}

void XlibBackend::setTouchpadEnabled(bool enable)
{
    if (!m_device) {
        return;
    }

    m_device->setEnabled(enable);

    // FIXME? This should not be needed, m_notifications should trigger
    // a propertyChanged signal when we enable/disable the touchpad,
    // that will Q_EMIT touchpadStateChanged, but for some reason
    // XlibNotifications is not getting the property change events
    // so we just Q_EMIT touchpadStateChanged from here as a workaround
    Q_EMIT touchpadStateChanged();
}

void XlibBackend::setTouchpadOff(TouchpadBackend::TouchpadOffState state)
{
    if (!m_device) {
        return;
    }

    int touchpadOff = 0;
    switch (state) {
    case TouchpadEnabled:
        touchpadOff = 0;
        break;
    case TouchpadFullyDisabled:
        touchpadOff = 1;
        break;
    case TouchpadTapAndScrollDisabled:
        touchpadOff = 2;
        break;
    default:
        qCritical() << "Unknown TouchpadOffState" << state;
        return;
    }

    m_device->setTouchpadOff(touchpadOff);
}

bool XlibBackend::isTouchpadAvailable()
{
    return m_device != nullptr;
}

bool XlibBackend::isTouchpadEnabled()
{
    if (!m_device) {
        return false;
    }

    return m_device->enabled();
}

TouchpadBackend::TouchpadOffState XlibBackend::getTouchpadOff()
{
    if (!m_device) {
        return TouchpadFullyDisabled;
    }
    int value = m_device->touchpadOff();
    switch (value) {
    case 0:
        return TouchpadEnabled;
    case 1:
        return TouchpadFullyDisabled;
    case 2:
        return TouchpadTapAndScrollDisabled;
    default:
        qCritical() << "Unknown TouchpadOff value" << value;
        return TouchpadFullyDisabled;
    }
}

void XlibBackend::touchpadDetached()
{
    qWarning() << "Touchpad detached";
    m_device.reset();
    Q_EMIT touchpadReset();
}

void XlibBackend::devicePlugged(int device)
{
    if (!m_device) {
        m_device.reset(findTouchpad());
        if (m_device) {
            qWarning() << "Touchpad reset";
            // We get called by m_notifications, need to use deleteLater
            m_notifications.release()->deleteLater();
            watchForEvents(m_keyboard != nullptr);
            Q_EMIT touchpadReset();
        }
    }
    if (!m_device || device != m_device->deviceId()) {
        Q_EMIT mousesChanged();
    }
}

void XlibBackend::propertyChanged(xcb_atom_t prop)
{
    if ((m_device && prop == m_device->touchpadOffAtom().atom()) || prop == m_enabledAtom.atom()) {
        Q_EMIT touchpadStateChanged();
    }
}

QStringList XlibBackend::listMouses(const QStringList &blacklist)
{
    int nDevices = 0;
    std::unique_ptr<XDeviceInfo, DeviceListDeleter> info(XListInputDevices(m_display.get(), &nDevices));
    QStringList list;
    for (XDeviceInfo *i = info.get(); i != info.get() + nDevices; i++) {
        if (m_device && i->id == static_cast<XID>(m_device->deviceId())) {
            continue;
        }
        if (i->use != IsXExtensionPointer && i->use != IsXPointer) {
            continue;
        }
        // type = KEYBOARD && use = Pointer means usb receiver for both keyboard
        // and mouse
        if (i->type != m_mouseAtom.atom() && i->type != m_keyboardAtom.atom()) {
            continue;
        }
        QString name(i->name);
        if (blacklist.contains(name, Qt::CaseInsensitive)) {
            continue;
        }
        PropertyInfo enabled(m_display.get(), i->id, m_enabledAtom.atom(), 0);
        if (enabled.value(0) == false) {
            continue;
        }
        list.append(name);
    }

    return list;
}

QList<QObject *> XlibBackend::getDevices() const
{
    QList<QObject *> touchpads;

#if HAVE_XORGLIBINPUT
    LibinputTouchpad *libinputtouchpad = dynamic_cast<LibinputTouchpad *>(m_device.get());
    if (libinputtouchpad) {
        touchpads.push_back(libinputtouchpad);
    }
#endif

    return touchpads;
}

void XlibBackend::watchForEvents(bool keyboard)
{
    if (!m_notifications) {
        m_notifications.reset(new XlibNotifications(m_display.get(), m_device ? m_device->deviceId() : XIAllDevices));
        connect(m_notifications.get(), &XlibNotifications::devicePlugged, this, &XlibBackend::devicePlugged);
        connect(m_notifications.get(), &XlibNotifications::touchpadDetached, this, &XlibBackend::touchpadDetached);
        connect(m_notifications.get(), &XlibNotifications::propertyChanged, this, &XlibBackend::propertyChanged);
    }

    if (keyboard == (m_keyboard != nullptr)) {
        return;
    }

    if (!keyboard) {
        m_keyboard.reset();
        return;
    }

    m_keyboard.reset(new XRecordKeyboardMonitor(m_display.get()));
    connect(m_keyboard.get(), &XRecordKeyboardMonitor::keyboardActivityStarted, this, &XlibBackend::keyboardActivityStarted);
    connect(m_keyboard.get(), &XRecordKeyboardMonitor::keyboardActivityFinished, this, &XlibBackend::keyboardActivityFinished);
}

#include "moc_xlibbackend.cpp"
