/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "inputdevice.h"

#include <QList>
#include <QObject>
#include <QVariantHash>

#include "backends/kwin_wl/kwin_wl_device.h"

#include <KConfigGroup>

namespace KWinDevices
{
class DevicesModel;
}

class InputBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<InputDevice *> inputDevices READ inputDevices NOTIFY inputDevicesChanged FINAL)
    Q_PROPERTY(QVariantMap buttonMapping READ buttonMapping WRITE setButtonMapping NOTIFY buttonMappingChanged FINAL)
    Q_PROPERTY(int buttonMappingCount READ buttonMappingCount NOTIFY buttonMappingChanged STORED false FINAL)

public:
    static void registerImplementationTypes(const char *uri);

    explicit InputBackend();
    ~InputBackend();

    bool save();
    bool load();
    bool defaults();
    bool isSaveNeeded() const;
    QString errorString() const;
    int deviceCount() const;
    QList<InputDevice *> inputDevices() const;

    QVariantMap buttonMapping() const;
    void setButtonMapping(const QVariantMap &mapping);

    // Because QML can't QVariantMap::count() on its own.
    int buttonMappingCount() const
    {
        return buttonMapping().count();
    }

Q_SIGNALS:
    void needsSaveChanged();

    void inputDevicesChanged();
    void deviceAdded(bool success);
    void deviceRemoved(int index);
    void buttonMappingChanged();

private:
    void findDevices();
    // all_of without short-circuiting is like for_each but with an AND-reduced accumulator.
    bool forAllDevices(bool (KWinWaylandDevice::*f)()) const;

    void onDeviceRowsInserted(const QModelIndex &parent, int first, int last);
    void onDeviceRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);

    static KConfigGroup mouseButtonRebindsConfigGroup();

    KWinDevices::DevicesModel *m_devicesModel;
    std::vector<std::unique_ptr<KWinWaylandDevice>> m_devices;
    QVariantMap m_buttonMapping;
    QVariantMap m_loadedButtonMapping;

    QString m_errorString;
};
