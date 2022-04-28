/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KWINWAYLANDBACKEND_H
#define KWINWAYLANDBACKEND_H

#include "inputbackend.h"

#include <QVector>

class QDBusInterface;

class KWinWaylandBackend : public InputBackend
{
    Q_OBJECT

    Q_PROPERTY(int deviceCount READ deviceCount CONSTANT)
    Q_PROPERTY(QVariantMap buttonMapping READ buttonMapping WRITE setButtonMapping NOTIFY buttonMappingChanged)

public:
    explicit KWinWaylandBackend(QObject *parent = nullptr);
    ~KWinWaylandBackend();

    bool applyConfig() override;
    bool getConfig() override;
    bool getDefaultConfig() override;
    bool isChangedConfig() const override;
    QString errorString() const override
    {
        return m_errorString;
    }

    virtual int deviceCount() const override
    {
        return m_devices.count();
    }
    virtual QVector<QObject *> getDevices() const override
    {
        return m_devices;
    }

    QVariantMap buttonMapping();
    void setButtonMapping(const QVariantMap &mapping);

Q_SIGNALS:
    void buttonMappingChanged();

private Q_SLOTS:
    void onDeviceAdded(QString);
    void onDeviceRemoved(QString);

private:
    void findDevices();

    QDBusInterface *m_deviceManager;
    QVector<QObject *> m_devices;
    QVariantMap m_buttonMapping;
    QVariantMap m_loadedButtonMapping;

    QString m_errorString = QString();
};

#endif // KWINWAYLANDBACKEND_H
