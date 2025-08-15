/*
    SPDX-FileCopyrightText: 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2014 Frederik Gladhorn <gladhorn@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <cmath>
#include <unistd.h>

#include "kaccess.h"

#include <QAction>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QLoggingCategory>
#include <QProcess>
#include <QTimer>
#include <QtGui/private/qtx11extras_p.h>

#include <KConfig>
#include <KConfigGroup>
#include <KDBusService>
#include <KGlobalAccel>
#include <KLocalizedString>
#include <KSharedConfig>

Q_LOGGING_CATEGORY(logKAccess, "kcm_kaccess")

KAccessApp::KAccessApp()
    : m_bellSettings(new BellSettings(this))
    , m_kdeglobals(QStringLiteral("kdeglobals"))
{
    auto service = new KDBusService(KDBusService::Unique, this);
    connect(service, &KDBusService::activateRequested, this, &KAccessApp::newInstance);

    QTimer::singleShot(0, this, &KAccessApp::readSettings);
}

void KAccessApp::newInstance()
{
    KSharedConfig::openConfig()->reparseConfiguration();
    m_bellSettings.load();
    readSettings();
}

void KAccessApp::readSettings()
{
    // select bell events
    XkbSelectEvents(QX11Info::display(), XkbUseCoreKbd, XkbBellNotifyMask, XkbBellNotifyMask);

    // deactivate system bell to allow playing our own sound
    XkbChangeEnabledControls(QX11Info::display(), XkbUseCoreKbd, XkbAudibleBellMask, 0);
}

struct xkb_any_ {
    uint8_t response_type;
    uint8_t xkbType;
    uint16_t sequence;
    xcb_timestamp_t time;
    uint8_t deviceID;
};

bool KAccessApp::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result);

    if (eventType == "xcb_generic_event_t") {
        xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
        if ((event->response_type & ~0x80) == XkbEventCode + xkb_opcode) {
            xkb_any_ *ev = reinterpret_cast<xkb_any_ *>(event);
            // Workaround for an XCB bug. xkbType comes from an EventType that is defined with bits, like
            // <item name="BellNotify">             <bit>8</bit>
            // while the generated XCB event type enum is defined as a bitmask, like
            //     XCB_XKB_EVENT_TYPE_BELL_NOTIFY = 256
            // This means if xbkType is 8, we need to set the 8th bit to 1, thus raising 2 to power of 8.
            // See also https://bugs.freedesktop.org/show_bug.cgi?id=51295
            const int eventType = pow(2, ev->xkbType);
            switch (eventType) {
            case XCB_XKB_EVENT_TYPE_BELL_NOTIFY:
                xkbBellNotify(reinterpret_cast<xcb_xkb_bell_notify_event_t *>(event));
                break;
            }
            return true;
        }
    }
    return false;
}

void KAccessApp::xkbBellNotify(xcb_xkb_bell_notify_event_t *event)
{
    // bail out if we should not really ring
    if (event->eventOnly)
        return;

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/org/kde/KWin/Effect/SystemBell1"),
                                                      QStringLiteral("org.kde.KWin.Effect.SystemBell1"),
                                                      QStringLiteral("triggerWindow"));

    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

void KAccessApp::setXkbOpcode(int opcode)
{
    xkb_opcode = opcode;
}

#include "moc_kaccess.cpp"
