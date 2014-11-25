/***************************************************************************
 *   Copyright (C) 2011, 2012, 2014 Albert Astals Cid <aacid@kde.org>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#include "ktouchpadenabler_daemon.h"

#include "settings.h"

#include <KGlobalAccel>
#include <KLocalizedString>
#include <KNotification>
#include <KPluginFactory>

#include <QAction>
#include <QX11Info>

#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>

class TouchpadEnablerDaemonPrivate : public QObject
{
Q_OBJECT
    public:
        TouchpadEnablerDaemonPrivate();
        ~TouchpadEnablerDaemonPrivate();

        bool isOk() const { return ok; }

    private Q_SLOTS:
        void toggle();
        void on();
        void off();
        
    private:
        bool getEnabled(bool *currentValue) const;
        void setEnabled(bool enabled, bool showNotification) const;
        
        Display *m_display;
        int m_deviceId;
        Atom m_enabledProperty;
        bool ok;
};

TouchpadEnablerDaemonPrivate::TouchpadEnablerDaemonPrivate()
{
    bool foundTouchpad = false;
    bool foundMoreThanOneTouchpad = false;
    ok = false;
    
    m_display = QX11Info::display();
    if (!m_display) {
        qWarning() << "Did not find a display to use. This should never happen, thus doing nothing. Please report a bug against ktouchpadenabler in http://bugs.kde.org";
        return;
    }
    
    Atom synapticsProperty = XInternAtom (m_display, "Synaptics Off", False);
    m_enabledProperty = XInternAtom (m_display, "Device Enabled", False);

    if (synapticsProperty && m_enabledProperty) {
        int nDevices;
        XIDeviceInfo *devices = XIQueryDevice(m_display, XIAllDevices, &nDevices);
        for (int i = 0; i < nDevices; ++i) {
            Atom realtype;
            int realformat;
            unsigned long nitems, bytes_after;
            unsigned char *data;
            if ((XIGetProperty (m_display, devices[i].deviceid, synapticsProperty, 0, 1, False, XA_INTEGER, &realtype, &realformat, &nitems, &bytes_after, &data) == Success) && (realtype != None)) {
                XFree (data);
                if ((XIGetProperty (m_display, devices[i].deviceid, m_enabledProperty, 0, 1, False, XA_INTEGER, &realtype, &realformat, &nitems, &bytes_after, &data) == Success) && (realtype != None)) {
                    XFree (data);
                    if (foundTouchpad) {
                        foundMoreThanOneTouchpad = true;
                    } else {
                        m_deviceId = devices[i].deviceid;
                        foundTouchpad = true;
                    }
                }
            }
        }
        if (devices) {
            XIFreeDeviceInfo(devices);
        }
    } else {
        qWarning() << "Could not get atoms for 'Synaptics Off' or 'Device Enabled'. This should never happen, thus doing nothing. Please report a bug against ktouchpadenabler in http://bugs.kde.org";
    }
    
    if (foundTouchpad) {
        if (!foundMoreThanOneTouchpad) {
            QAction *toggleAction = new QAction(i18n("Toggle touchpad enabled state"), this);
            toggleAction->setObjectName("ktouchpadenabler_toggle");
            connect(toggleAction, &QAction::triggered, this, &TouchpadEnablerDaemonPrivate::toggle);
            bool okToggle = KGlobalAccel::setGlobalShortcut(toggleAction, QKeySequence(Qt::Key_TouchpadToggle));
            if (!okToggle) {
                qDebug() << "Could set global shortcut to Qt::Key_TouchpadToggle. You probably have some other program using it, if you are sure you don't have any, please report a bug against ktouchpadenabler in http://bugs.kde.org";
            }

            QAction *onAction = new QAction(i18n("Set touchpad on"), this);
            onAction->setObjectName("ktouchpadenabler_on");
            connect(onAction, &QAction::triggered, this, &TouchpadEnablerDaemonPrivate::on);
            bool okOn = KGlobalAccel::setGlobalShortcut(onAction, QKeySequence(Qt::Key_TouchpadOn));
            if (!okOn) {
                qDebug() << "Could set global shortcut to Qt::Key_TouchpadOn. You probably have some other program using it, if you are sure you don't have any, please report a bug against ktouchpadenabler in http://bugs.kde.org";
            }

            QAction *offAction = new QAction(i18n("Set touchpad off"), this);
            offAction->setObjectName("ktouchpadenabler_off");
            connect(offAction, &QAction::triggered, this, &TouchpadEnablerDaemonPrivate::off);
            bool okOff = KGlobalAccel::setGlobalShortcut(offAction, QKeySequence(Qt::Key_TouchpadOff));
            if (!okOff) {
                qDebug() << "Could set global shortcut to Qt::Key_TouchpadOff. You probably have some other program using it, if you are sure you don't have any, please report a bug against ktouchpadenabler in http://bugs.kde.org";
            }

            ok = okToggle || okOn || okOff;
            if (ok) {
                bool currentlyEnabled;
                if (getEnabled(&currentlyEnabled)) {
                    const bool newValue = ktouchpadenabler::Settings::self()->touchpadEnabled();
                    if (newValue != currentlyEnabled) {
                        setEnabled(newValue, false);
                    }
                }
            }
        } else {
            KNotification *notification = KNotification::event(KNotification::Warning, i18n("Touchpad status"), i18n("More than one touchpad detected. Touchpad Enabler Daemon does not handle this configuration"));
            notification->sendEvent();
        }
    } else {
        qDebug() << "Did not find a touchpad. If you have one, please report a bug against ktouchpadenabler in http://bugs.kde.org";
    }
}

TouchpadEnablerDaemonPrivate::~TouchpadEnablerDaemonPrivate()
{
}

void TouchpadEnablerDaemonPrivate::toggle()
{
    bool currentlyEnabled;
    if (getEnabled(&currentlyEnabled)) {
        bool newValue = !currentlyEnabled;
        setEnabled(newValue, true);
    }
}

void TouchpadEnablerDaemonPrivate::on()
{
    bool currentlyEnabled;
    if (getEnabled(&currentlyEnabled) && !currentlyEnabled) {
        setEnabled(true, true);
    }
}

void TouchpadEnablerDaemonPrivate::off()
{
    bool currentlyEnabled;
    if (getEnabled(&currentlyEnabled) && currentlyEnabled) {
        setEnabled(false, true);
    }
}

bool TouchpadEnablerDaemonPrivate::getEnabled(bool *enabled) const
{
    Atom realtype;
    int realformat;
    unsigned long nitems, bytes_after;
    unsigned char *value;
    if ((XIGetProperty(m_display, m_deviceId, m_enabledProperty, 0, 1, False, XA_INTEGER, &realtype, &realformat, &nitems, &bytes_after, &value) == Success) && (realtype != None)) {
        *enabled = (*value == 1);
        XFree(value);
        return true;
    } else {
        return false;
    }
}

void TouchpadEnablerDaemonPrivate::setEnabled(bool enabled, bool showNotification) const
{
    unsigned char newValue = enabled ? 1 : 0;
    XIChangeProperty(m_display, m_deviceId, m_enabledProperty, XA_INTEGER, 8, PropModeReplace, &newValue, 1);
    XFlush(m_display);

    if (showNotification) {
        KNotification *notification = KNotification::event(KNotification::Notification, i18n("Touchpad status"), enabled ? i18n("Touchpad enabled") : i18n("Touchpad disabled"));
        notification->sendEvent();

        ktouchpadenabler::Settings::self()->setTouchpadEnabled(newValue);
        ktouchpadenabler::Settings::self()->save();
    }
}

TouchpadEnablerDaemon::TouchpadEnablerDaemon(QObject *parent, const QList<QVariant>&)
    : KDEDModule(parent)
{
    d = new TouchpadEnablerDaemonPrivate();
    
    if (!d->isOk()) {
        delete d;
        d = 0;
    }
}

TouchpadEnablerDaemon::~TouchpadEnablerDaemon()
{
    delete d;
}

K_PLUGIN_FACTORY(TouchpadEnablerFactory, registerPlugin<TouchpadEnablerDaemon>();)
K_EXPORT_PLUGIN(TouchpadEnablerFactory("ktouchpadenabler"))

#include "ktouchpadenabler_daemon.moc"
