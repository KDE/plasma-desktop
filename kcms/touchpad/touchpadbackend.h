/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
