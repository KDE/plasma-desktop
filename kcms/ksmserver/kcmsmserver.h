/*
 *  kcmsmserver.h
 *  Copyright (c) 2000 Oswald Buddenhagen <ob6@inf.tu-dresden.de>
 *
 *  based on kcmtaskbar.h
 *  Copyright (c) 2000 Kurt Granroth <granroth@kde.org>
 *
 *  Copyright (c) 2019 Kevin Ottens <kevin.ottens@enioka.com>
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
 */
#pragma once

#include <KQuickAddons/ManagedConfigModule>

class QAction;

class SMServerConfigImpl;

class OrgFreedesktopLogin1ManagerInterface;

/// KCM handling the desktop session and in particular the login and logout
/// handling.
class SMServerConfig : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

    /// True if we are in an UEFI system.
    Q_PROPERTY(bool isUefi READ isUefi CONSTANT)

    /// Config telling if we should restart in the setup screen next time we boot.
    Q_PROPERTY(bool restartInSetupScreen READ restartInSetupScreen WRITE setRestartInSetupScreen NOTIFY restartInSetupScreenChanged)

    /// Error message, empty if there is no error
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

    /// Can setup the firmware
    Q_PROPERTY(bool canFirmwareSetup READ canFirmwareSetup CONSTANT)

public:
    explicit SMServerConfig(QObject *parent = nullptr, const QVariantList &list = QVariantList());
    ~SMServerConfig() override;

    bool isUefi() const;

    bool restartInSetupScreen() const;
    void setRestartInSetupScreen(bool isUefi);

    QString error() const;

    bool canFirmwareSetup() const;

    /// Tell the computer to reboot.
    Q_INVOKABLE void reboot();

    void defaults() override;

Q_SIGNALS:
    void isUefiChanged();
    void restartInSetupScreenChanged();
    void errorChanged();

private:
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    void checkFirmwareSetupRequested();

    OrgFreedesktopLogin1ManagerInterface *m_login1Manager = nullptr;
    QAction *m_rebootNowAction = nullptr;
    bool m_isUefi = false;
    bool m_restartInSetupScreen = false;
    bool m_restartInSetupScreenInitial = false;
    bool m_canFirmwareSetup = false;
    QString m_error;
};
