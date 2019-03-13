/*
 *  kcmsmserver.cpp
 *  Copyright (c) 2000,2002 Oswald Buddenhagen <ossi@kde.org>
 *
 *  based on kcmtaskbar.cpp
 *  Copyright (c) 2000 Kurt Granroth <granroth@kde.org>
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
//Added by qt3to4:
#include <QVBoxLayout>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kworkspace.h>
#include <qregexp.h>
#include <kdesktopfile.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <QApplication>
#include <QDBusInterface>
#include <QLineEdit>

#include "kcmsmserver.h"
#include "smserverconfigimpl.h"
#include <KPluginFactory>
#include <KPluginLoader>
#include <KLocalizedString>

#include "kworkspace.h"

#include "login1_manager.h"

K_PLUGIN_FACTORY(SMSFactory, registerPlugin<SMServerConfig>();)

SMServerConfig::SMServerConfig(QWidget *parent, const QVariantList &args)
  : KCModule(parent, args)
  , m_login1Manager(new OrgFreedesktopLogin1ManagerInterface(QStringLiteral("org.freedesktop.login1"),
                                                             QStringLiteral("/org/freedesktop/login1"),
                                                             QDBusConnection::systemBus(),
                                                             this))
{
    setQuickHelp( i18n("<h1>Session Manager</h1>"
    " You can configure the session manager here."
    " This includes options such as whether or not the session exit (logout)"
    " should be confirmed, whether the session should be restored again when logging in"
    " and whether the computer should be automatically shut down after session"
    " exit by default."));

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(0, 0, 0, 0);
    dialog = new SMServerConfigImpl(this);
    connect(dialog, SIGNAL(changed()), SLOT(changed()));

    initFirmwareSetup();
    checkFirmwareSetupRequested();

    topLayout->addWidget(dialog);
}

void SMServerConfig::initFirmwareSetup()
{
    m_rebootNowAction = new QAction(QIcon::fromTheme(QStringLiteral("system-reboot")), i18n("Restart Now"));
    connect(m_rebootNowAction, &QAction::triggered, this, [] {
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmNo, KWorkSpace::ShutdownTypeReboot);
    });

    connect(dialog->firmwareSetupCheck, &QCheckBox::clicked, this, [this](bool enable) {
        dialog->firmwareSetupMessageWidget->removeAction(m_rebootNowAction);
        dialog->firmwareSetupMessageWidget->animatedHide();

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

            KMessageWidget *message = dialog->firmwareSetupMessageWidget;

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
            dialog->firmwareSetupBox->setTitle(i18n("UEFI Setup"));
            dialog->firmwareSetupCheck->setText(i18n("Enter UEFI setup on next restart"));
        }

        dialog->firmwareSetupBox->setVisible(true);
    }
}

void SMServerConfig::checkFirmwareSetupRequested()
{
    dialog->firmwareSetupCheck->setChecked(m_login1Manager->property("RebootToFirmwareSetup").toBool());
}

void SMServerConfig::load()
{
  KConfigGroup c(KSharedConfig::openConfig(QStringLiteral("ksmserverrc"), KConfig::NoGlobals),
                 QStringLiteral("General"));
  dialog->confirmLogoutCheck->setChecked(c.readEntry("confirmLogout", true));
  bool en = c.readEntry("offerShutdown", true);
  dialog->offerShutdownCheck->setChecked(en);
  dialog->sdGroup->setEnabled(en);

  QString s = c.readEntry( "loginMode" );
  if ( s == QStringLiteral("default") )
      dialog->emptySessionRadio->setChecked(true);
  else if ( s == QStringLiteral("restoreSavedSession") )
      dialog->savedSessionRadio->setChecked(true);
  else // "restorePreviousLogout"
      dialog->previousSessionRadio->setChecked(true);

  switch (c.readEntry("shutdownType", int(KWorkSpace::ShutdownTypeNone))) {
  case int(KWorkSpace::ShutdownTypeHalt):
    dialog->haltRadio->setChecked(true);
    break;
  case int(KWorkSpace::ShutdownTypeReboot):
    dialog->rebootRadio->setChecked(true);
    break;
  default:
    dialog->logoutRadio->setChecked(true);
    break;
  }
  dialog->excludeLineedit->setText( c.readEntry("excludeApps"));

  emit changed(false);
}

void SMServerConfig::save()
{
  KConfig c(QStringLiteral("ksmserverrc"), KConfig::NoGlobals);
  KConfigGroup group = c.group(QStringLiteral("General"));
  group.writeEntry( "confirmLogout", dialog->confirmLogoutCheck->isChecked());
  group.writeEntry( "offerShutdown", dialog->offerShutdownCheck->isChecked());
  QString s = QStringLiteral("restorePreviousLogout");
  if ( dialog->emptySessionRadio->isChecked() )
      s = QStringLiteral("default");
  else if ( dialog->savedSessionRadio->isChecked() )
      s = QStringLiteral("restoreSavedSession");
  group.writeEntry( "loginMode", s );

  group.writeEntry( "shutdownType",
                 dialog->haltRadio->isChecked() ?
                   int(KWorkSpace::ShutdownTypeHalt) :
                   dialog->rebootRadio->isChecked() ?
                     int(KWorkSpace::ShutdownTypeReboot) :
                     int(KWorkSpace::ShutdownTypeNone));
  group.writeEntry("excludeApps", dialog->excludeLineedit->text());
  c.sync();
# if 0
  // update the k menu if necessary
  QDBusInterface kicker("org.kde.kicker", "/kicker", "org.kde.kicker");
  kicker.call("configure");
#endif
}

void SMServerConfig::defaults()
{
  dialog->previousSessionRadio->setChecked(true);
  dialog->confirmLogoutCheck->setChecked(true);
  dialog->offerShutdownCheck->setChecked(true);
  dialog->sdGroup->setEnabled(true);
  dialog->logoutRadio->setChecked(true);
  dialog->excludeLineedit->clear();
}

#include "kcmsmserver.moc"
