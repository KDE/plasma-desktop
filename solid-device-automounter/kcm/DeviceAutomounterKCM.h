/**************************************************************************
 *   Copyright (C) 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>  *
 *   Copyright (C) 2015 Kai Uwe Broulik <kde@privat.broulik.de>            *
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

#ifndef DEVICEAUTOMOUNTERKCM_H
#define DEVICEAUTOMOUNTERKCM_H

#include <KCModule>

#include "ui_DeviceAutomounterKCM.h"

class DeviceModel;
class AutomounterSettings;

class DeviceAutomounterKCM : public KCModule, public Ui::DeviceAutomounterKCM
{
    Q_OBJECT

public:
    explicit DeviceAutomounterKCM(QWidget *parent = nullptr, const QVariantList &args = QVariantList());
    ~DeviceAutomounterKCM() override;

public Q_SLOTS:
    void load() override;
    void save() override;

private Q_SLOTS:
    void updateForgetDeviceButton();
    void forgetSelectedDevices();

private:
    void saveLayout();
    void loadLayout();

    AutomounterSettings *m_settings;
    DeviceModel *m_devices;
};

#endif
