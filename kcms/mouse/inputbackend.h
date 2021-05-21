/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INPUTBACKEND_H
#define INPUTBACKEND_H

#include <QObject>
#include <QVariantHash>
#include <QVector>

enum class InputBackendMode {
    KWinWayland = 0,
    XLibinput = 1,
    XEvdev = 2,
};

class InputBackend : public QObject
{
    Q_OBJECT

protected:
    explicit InputBackend(QObject *parent)
        : QObject(parent)
    {
    }
    InputBackendMode m_mode;

public:
    static InputBackend *implementation(QObject *parent = nullptr);

    InputBackendMode mode()
    {
        return m_mode;
    }

    virtual void kcmInit()
    {
    }

    virtual bool isValid() const
    {
        return false;
    }

    virtual void load()
    {
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

    virtual QString errorString() const
    {
        return QString();
    }

    virtual int deviceCount() const
    {
        return 0;
    }
    virtual QVector<QObject *> getDevices() const
    {
        return QVector<QObject *>();
    }

Q_SIGNALS:
    void deviceAdded(bool success);
    void deviceRemoved(int index);
};

#endif // INPUTBACKEND_H
