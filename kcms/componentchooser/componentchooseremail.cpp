/***************************************************************************
                          componentchooseremail.cpp
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#include "componentchooseremail.h"

#include <kemailsettings.h>
#include <kopenwithdialog.h>

#include <KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>
#include <KService>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QUrl>
#include <QFile>
#include <QCheckBox>

// for chmod:
#include <sys/types.h>
#include <sys/stat.h>
#include <QStandardPaths>


CfgEmailClient::CfgEmailClient(QWidget *parent)
    : QWidget(parent), Ui::EmailClientConfig_UI(), CfgPlugin()
{
    setupUi( this );

    pSettings = new KEMailSettings();

    connect(kmailCB, &QRadioButton::toggled, this, &CfgEmailClient::configChanged);
    connect(txtEMailClient, &KLineEdit::textChanged, this, &CfgEmailClient::configChanged);
#ifdef Q_OS_UNIX
    connect(chkRunTerminal, &QCheckBox::clicked, this, &CfgEmailClient::configChanged);
#else
    chkRunTerminal->hide();
#endif
    connect(btnSelectEmail, &QToolButton::clicked, this, &CfgEmailClient::selectEmailClient);
}

CfgEmailClient::~CfgEmailClient() {
    delete pSettings;
}

void CfgEmailClient::defaults()
{
    kmailCB->setChecked(true);
    txtEMailClient->clear();
    chkRunTerminal->setChecked(false);
}

bool CfgEmailClient::isDefaults() const
{
	return kmailCB->isChecked();
}

void CfgEmailClient::load(KConfig *)
{
    QString emailClient = pSettings->getSetting(KEMailSettings::ClientProgram);
    bool useKMail = (emailClient.isEmpty());

    kmailCB->setChecked(useKMail);
    otherCB->setChecked(!useKMail);
    txtEMailClient->setText(emailClient);
    txtEMailClient->setFixedHeight(txtEMailClient->sizeHint().height());
    chkRunTerminal->setChecked((pSettings->getSetting(KEMailSettings::ClientTerminal) == QLatin1String("true")));

    emit changed(false);

}

void CfgEmailClient::configChanged()
{
    emit changed(true);
}

void CfgEmailClient::selectEmailClient()
{
    QList<QUrl> urlList;
    KOpenWithDialog dlg(urlList, i18n("Select preferred email client:"), QString(), this);
    // hide "Do not &close when command exits" here, we don't need it for a mail client
    dlg.hideNoCloseOnExit();
    if (dlg.exec() != QDialog::Accepted) return;
    QString client = dlg.text();
    m_emailClientService = dlg.service();

    // get the preferred Terminal Application
    KConfigGroup confGroup( KSharedConfig::openConfig(), QStringLiteral("General") );
    QString preferredTerminal = confGroup.readPathEntry("TerminalApplication", QStringLiteral("konsole"));
    preferredTerminal += QLatin1String(" -e ");

    int len = preferredTerminal.length();
    bool b = client.left(len) == preferredTerminal;
    if (b) client = client.mid(len);
    if (!client.isEmpty())
    {
        chkRunTerminal->setChecked(b);
        txtEMailClient->setText(client);
    }
}


void CfgEmailClient::save(KConfig *)
{
    if (kmailCB->isChecked())
    {
        pSettings->setSetting(KEMailSettings::ClientProgram, QString());
        pSettings->setSetting(KEMailSettings::ClientTerminal, QStringLiteral("false"));
    }
    else
    {
        pSettings->setSetting(KEMailSettings::ClientProgram, txtEMailClient->text());
        pSettings->setSetting(KEMailSettings::ClientTerminal, (chkRunTerminal->isChecked()) ? "true" : "false");
    }

    // Save the default email client in mimeapps.list into the group [Default Applications]
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (profile->isConfigWritable(true)) {
        KConfigGroup defaultApp(profile, "Default Applications");
        if (kmailCB->isChecked()) {
            QString kmailDesktop = QStringLiteral("org.kde.kmail.desktop");
            if (KService::serviceByDesktopName(QStringLiteral("org.kde.kmail2"))) {
                kmailDesktop = QStringLiteral("org.kde.kmail2.desktop");
            }
            defaultApp.writeXdgListEntry("x-scheme-handler/mailto", QStringList(kmailDesktop));
        } else if (m_emailClientService) {
            defaultApp.writeXdgListEntry("x-scheme-handler/mailto", QStringList(m_emailClientService->storageId()));
        }
        profile->sync();
    }

    // insure proper permissions -- contains sensitive data
    QString cfgName(QStandardPaths::locate(QStandardPaths::ConfigLocation, QStringLiteral("emails")));
    if (!cfgName.isEmpty())
        ::chmod(QFile::encodeName(cfgName), 0600);
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/Component"), QStringLiteral("org.kde.Kcontrol"), QStringLiteral("KDE_emailSettingsChanged") );
    QDBusConnection::sessionBus().send(message);
    emit changed(false);
}

