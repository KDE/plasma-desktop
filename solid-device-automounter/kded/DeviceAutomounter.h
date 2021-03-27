/***************************************************************************
 *   Copyright (C) 2009 by Trever Fischer <wm161@wm161.net>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#ifndef DEVICEAUTOMOUNTER_H
#define DEVICEAUTOMOUNTER_H

#include "AutomounterSettings.h"
#include <KDEDModule>
#include <Solid/Device>

class DeviceAutomounter : public KDEDModule
{
    Q_OBJECT
public:
    explicit DeviceAutomounter(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~DeviceAutomounter() override;

private Q_SLOTS:
    void init();
    void deviceAdded(const QString &udi);
    void deviceMountChanged(bool accessible, const QString &udi);

private:
    void automountDevice(Solid::Device &dev, AutomounterSettings::AutomountType type);

    AutomounterSettings *m_settings;
};

#endif
