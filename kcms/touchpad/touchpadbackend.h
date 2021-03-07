/*
 * Copyright 2017 Roman Gilg <subdiff@gmail.com>
 * Copyright 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TOUCHPADBACKEND_H
#define TOUCHPADBACKEND_H

#include <QObject>
#include <QVariantHash>
#include <QVector>

enum class TouchpadInputBackendMode {
    Unset = 0,
    WaylandLibinput = 1,
    XLibinput = 2,
    XSynaptics = 3,
};

void touchpadApplySavedConfig();

class Q_DECL_EXPORT TouchpadBackend : public QObject
{
    Q_OBJECT

protected:
    explicit TouchpadBackend(QObject *parent)
        : QObject(parent)
        , m_mode(TouchpadInputBackendMode::Unset)
    {
    }
    void setMode(TouchpadInputBackendMode mode);

public:
    static TouchpadBackend *implementation();

    TouchpadInputBackendMode getMode() const
    {
        return m_mode;
    }

    virtual bool applyConfig(const QVariantHash &)
    {
        return false;
    }
    virtual bool getConfig(QVariantHash &)
    {
        return false;
    }

    virtual bool applyConfig()
    {
        return false;
    }
    virtual bool getConfig()
    {
        return false;
    }
    virtual bool getDefaultConfig()
    {
        return false;
    }
    virtual bool isChangedConfig() const
    {
        return false;
    }

    virtual QStringList supportedParameters() const
    {
        return QStringList();
    }
    virtual QString errorString() const
    {
        return QString();
    }

    virtual QVector<QObject *> getDevices() const
    {
        return QVector<QObject *>();
    }
    virtual int touchpadCount() const
    {
        return 0;
    }

    enum TouchpadOffState {
        TouchpadEnabled,
        TouchpadTapAndScrollDisabled,
        TouchpadFullyDisabled,
    };
    virtual void setTouchpadOff(TouchpadOffState)
    {
    }
    virtual TouchpadOffState getTouchpadOff()
    {
        return TouchpadFullyDisabled;
    }

    virtual bool isTouchpadAvailable()
    {
        return false;
    }
    virtual bool isTouchpadEnabled()
    {
        return false;
    }
    virtual void setTouchpadEnabled(bool)
    {
    }

    virtual void watchForEvents(bool /*keyboard*/)
    {
    }

    virtual QStringList listMouses(const QStringList & /*blacklist*/)
    {
        return QStringList();
    }

private:
    TouchpadInputBackendMode m_mode;

Q_SIGNALS:
    void touchpadStateChanged();
    void mousesChanged();
    void touchpadReset();
    void keyboardActivityStarted();
    void keyboardActivityFinished();

    void touchpadAdded(bool success);
    void touchpadRemoved(int index);
};

#endif // TOUCHPADBACKEND_H
