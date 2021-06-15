/*
    SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef UDEV_HELPER_H_
#define UDEV_HELPER_H_

#include <QObject>

class UdevDeviceNotifier : public QObject
{
    Q_OBJECT

public:
    explicit UdevDeviceNotifier(QObject *parent = nullptr);
    ~UdevDeviceNotifier() override;

Q_SIGNALS:
    void newKeyboardDevice();
    void newPointerDevice();

private:
    void init();
    void socketActivated();

    struct udev *m_udev;
    struct udev_monitor *m_monitor;
};

#endif // UDEV_HELPER_H_
