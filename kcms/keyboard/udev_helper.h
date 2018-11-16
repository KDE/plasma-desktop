/*
 *  Copyright (C) 2015 David Rosca (nowrep@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
