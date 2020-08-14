/*
 *  kcmsmserver.cpp
 *  Copyright (c) 2000,2002 Oswald Buddenhagen <ossi@kde.org>
 *
 *  based on kcmtaskbar.cpp
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

#include <QAction>
#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QCheckBox>
#include <QFileInfo>

#include <QVBoxLayout>

#include <kworkspace.h>
#include <QRegExp>
#include <KDesktopFile>
#include <KProcess>
#include <KMessageBox>
#include <QApplication>
#include <QDBusInterface>
#include <QLineEdit>

#include "kcmsmserver.h"
#include "smserversettings.h"
#include "smserverdata.h"
#include "ui_smserverconfigdlg.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KLocalizedString>

#include <sessionmanagement.h>

#include "login1_manager.h"

K_PLUGIN_FACTORY(SMSFactory, registerPlugin<SMServerConfig>(); registerPlugin<SMServerData>();)

SMServerConfig::SMServerConfig(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args)
  , ui(new Ui::SMServerConfigDlg)
  , m_data(new SMServerData(this))
  , m_login1Manager(new OrgFreedesktopLogin1ManagerInterface(QStringLiteral("org.freedesktop.login1"),
                                                             QStringLiteral("/org/freedesktop/login1"),
                                                             QDBusConnection::systemBus(),
                                                             this))
{
    ui->setupUi(this);

    setQuickHelp( i18n("<h1>Session Manager</h1>"
    " You can configure the session manager here."
    " This includes options such as whether or not the session exit (logout)"
    " should be confirmed, whether the session should be restored again when logging in"
    " and whether the computer should be automatically shut down after session"
    " exit by default."));

    ui->firmwareSetupBox->hide();
    ui->firmwareSetupMessageWidget->hide();

    initFirmwareSetup();
    checkFirmwareSetupRequested();

    addConfig(m_data->settings(), this);
}

SMServerConfig::~SMServerConfig() = default;

void SMServerConfig::initFirmwareSetup()
{
    m_rebootNowAction = new QAction(QIcon::fromTheme(QStringLiteral("system-reboot")), i18n("Restart Now"));
    connect(m_rebootNowAction, &QAction::triggered, this, [this] {
        auto sm = new SessionManagement(this);
        auto doShutdown=[sm]() {
            sm->requestReboot();
            delete sm;
        };
        if (sm->state() == SessionManagement::State::Loading) {
            connect(sm, &SessionManagement::stateChanged, this, doShutdown);
        } else {
            doShutdown();
        }
    });

    connect(ui->firmwareSetupCheck, &QCheckBox::clicked, this, [this](bool enable) {
        ui->firmwareSetupMessageWidget->removeAction(m_rebootNowAction);
        ui->firmwareSetupMessageWidget->animatedHide();

        QDBusMessage message = QDBusMessage::createMethodCall(m_login1Manager->service(),
                                                              m_login1Manager->path(),
                                                              m_login1Manager->interface(),
                                                              QStringLiteral("SetRebootToFirmwareSetup"));

        message.setArguments({enable});
        // This cannot be set through a generated DBus interface, so we have to create the message ourself.
        message.setInteractiveAuthorizationAllowed(true);

        QDBusPendingReply<void> call = m_login1Manager->connection().asyncCall(message);
        QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(call, this);
        connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [this, enable](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<void> reply = *watcher;
            watcher->deleteLater();

            checkFirmwareSetupRequested();

            KMessageWidget *message = ui->firmwareSetupMessageWidget;

            if (reply.isError()) {
                // User likely canceled the PolKit prompt, don't show an error in this case
                if (reply.error().type() != QDBusError::AccessDenied) {
                    message->setMessageType(KMessageWidget::Error);
                    message->setText(i18n("Failed to request restart to firmware setup: %1", reply.error().message()));
                    message->animatedShow();
                }
                return;
            }

            if (!enable) {
                return;
            }

            message->setMessageType(KMessageWidget::Information);
            if (m_isUefi) {
                message->setText(i18n("Next time the computer is restarted, it will enter the UEFI setup screen."));
            } else {
                message->setText(i18n("Next time the computer is restarted, it will enter the firmware setup screen."));
            }
            message->addAction(m_rebootNowAction);
            message->animatedShow();
        });
    });

    const QString canFirmareSetup = m_login1Manager->CanRebootToFirmwareSetup().value();
    if (canFirmareSetup == QLatin1String("yes") || canFirmareSetup == QLatin1String("challenge")) {
        // now check whether we're UEFI to provide a more descriptive button label
        if (QFileInfo(QStringLiteral("/sys/firmware/efi")).isDir()) {
            m_isUefi = true;
            ui->firmwareSetupBox->setTitle(i18n("UEFI Setup"));
            ui->firmwareSetupCheck->setText(i18n("Enter UEFI setup on next restart"));
        }

        ui->firmwareSetupBox->setVisible(true);
    }
}

void SMServerConfig::checkFirmwareSetupRequested()
{
    ui->firmwareSetupCheck->setChecked(m_login1Manager->property("RebootToFirmwareSetup").toBool());
}

#include "kcmsmserver.moc"
