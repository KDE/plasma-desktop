/***************************************************************************
                          componentchooser.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License verstion 2 as    *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#include "componentchooserterminal.h"

#include <ktoolinvocation.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCheckBox>

#include <kapplication.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kopenwithdialog.h>
#include <kconfig.h>

#include <kmimetypetrader.h>
#include <kurlrequester.h>
#include <kconfiggroup.h>
#include <KLocalizedString>
#include <KGlobalSettings>

#include <QUrl>

#include "../migrationlib/kdelibs4config.h"

CfgTerminalEmulator::CfgTerminalEmulator(QWidget *parent)
    : QWidget(parent), Ui::TerminalEmulatorConfig_UI(), CfgPlugin()
{
	setupUi(this);
	connect(terminalLE, &QLineEdit::textChanged, this, &CfgTerminalEmulator::configChanged);
	connect(terminalCB, &QRadioButton::toggled, this, &CfgTerminalEmulator::configChanged);
	connect(otherCB, &QRadioButton::toggled, this, &CfgTerminalEmulator::configChanged);
	connect(btnSelectTerminal, &QToolButton::clicked, this, &CfgTerminalEmulator::selectTerminalApp);

}

CfgTerminalEmulator::~CfgTerminalEmulator() {
}

void CfgTerminalEmulator::configChanged()
{
	emit changed(true);
}

void CfgTerminalEmulator::defaults()
{
	load(nullptr);
}


void CfgTerminalEmulator::load(KConfig *) {
        KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), "General");
	QString terminal = config.readPathEntry("TerminalApplication",QStringLiteral("konsole"));
	if (terminal == QLatin1String("konsole"))
	{
	   terminalLE->setText(QStringLiteral("xterm"));
	   terminalCB->setChecked(true);
	}
	else
	{
	  terminalLE->setText(terminal);
	  otherCB->setChecked(true);
	}

	emit changed(false);
}

void CfgTerminalEmulator::save(KConfig *)
{
	KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
	KConfigGroup config(profile, QStringLiteral("General"));
	const QString terminal = terminalCB->isChecked() ? QStringLiteral("konsole") : terminalLE->text();
	config.writePathEntry("TerminalApplication", terminal); // KConfig::Normal|KConfig::Global);

	config.sync();
        Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("General"), "kdeglobals");

	KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged);

        QDBusMessage message  = QDBusMessage::createMethodCall(QStringLiteral("org.kde.klauncher5"),
                                                      QStringLiteral("/KLauncher"),
                                                      QStringLiteral("org.kde.KLauncher"),
                                                      QStringLiteral("reparseConfiguration"));
        QDBusConnection::sessionBus().send(message);
	emit changed(false);
}

void CfgTerminalEmulator::selectTerminalApp()
{
	QList<QUrl> urlList;
	KOpenWithDialog dlg(urlList, i18n("Select preferred terminal application:"), QString(), this);
	// hide "Run in &terminal" here, we don't need it for a Terminal Application
	dlg.hideRunInTerminal();
	if (dlg.exec() != QDialog::Accepted) return;
	QString client = dlg.text();

	if (!client.isEmpty())
	{
		terminalLE->setText(client);
	}
}
// vim: sw=4 ts=4 noet
