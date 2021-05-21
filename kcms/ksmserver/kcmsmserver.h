/*
    kcmsmserver.h
    SPDX-FileCopyrightText: 2000 Oswald Buddenhagen <ob6@inf.tu-dresden.de>

    based on kcmtaskbar.h
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2019 Kevin Ottens <kevin.ottens@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
