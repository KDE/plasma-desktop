/***************************************************************************
                          componentchooser.cpp  -  description
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger <jowenn@kde.org>
    copyright            : (C) 2020 by Méven Car <meven.car@enioka.com>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation                             *
 *                                                                         *
 ***************************************************************************/

#include "componentchooserterminal.h"
#include "terminal_settings.h"

#include <ktoolinvocation.h>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QCheckBox>

#include <kmessagebox.h>
#include <kopenwithdialog.h>
#include <kconfig.h>

#include <kmimetypetrader.h>
#include <kurlrequester.h>
#include <kconfiggroup.h>
#include <KLocalizedString>
#include <KServiceTypeTrader>

#include <QUrl>

CfgTerminalEmulator::CfgTerminalEmulator(QWidget *parent)
    : QWidget(parent), Ui::TerminalEmulatorConfig_UI(), CfgPlugin()
{
    setupUi(this);
    connect(terminalCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgTerminalEmulator::selectTerminalEmulator);
}

CfgTerminalEmulator::~CfgTerminalEmulator() {
}

void CfgTerminalEmulator::selectTerminalEmulator(int index)
{
    if (index == terminalCombo->count() - 1) {
        selectTerminalApp();
    } else {
        emit changed(m_currentIndex != index);
    }
}

void CfgTerminalEmulator::defaults()
{
    if (m_konsoleIndex != -1) {
        terminalCombo->setCurrentIndex(m_konsoleIndex);
    }
}

bool CfgTerminalEmulator::isDefaults() const
{
    return m_konsoleIndex == -1 || m_konsoleIndex == terminalCombo->currentIndex();
}

void CfgTerminalEmulator::load(KConfig *)
{
	TerminalSettings settings;
    const QString terminal = settings.terminalApplication();

    m_currentIndex = -1;
    terminalCombo->clear();

    const auto constraint = QStringLiteral("'TerminalEmulator' in Categories AND (not exist NoDisplay OR NoDisplay == false)");
    const auto terminalEmulators = KServiceTypeTrader::self()->query(QStringLiteral("Application"), constraint);
    for (const auto &service : terminalEmulators) {
        terminalCombo->addItem(QIcon::fromTheme(service->icon()), service->name(), service->exec());

        if (!terminal.isEmpty() && service->exec() == terminal) {
            terminalCombo->setCurrentIndex(terminalCombo->count() - 1);
            m_currentIndex = terminalCombo->count() - 1;
        }
        if (service->exec() == QStringLiteral("konsole")) {
            m_konsoleIndex = terminalCombo->count() - 1;
        }
    }

    if (!terminal.isEmpty() && m_currentIndex == -1) {
        // we have a terminal specified by the user
        terminalCombo->addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), terminal, terminal);
        terminalCombo->setCurrentIndex(terminalCombo->count() - 1);
        m_currentIndex = terminalCombo->count() - 1;
    }

    // add a other option to add a new terminal emulator with KOpenWithDialog
    terminalCombo->addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), i18n("Other..."), QStringLiteral());

	emit changed(false);
}

void CfgTerminalEmulator::save(KConfig *)
{
    const QString terminal = terminalCombo->currentData().toString();
    m_currentIndex = terminalCombo->currentIndex();

	TerminalSettings settings;
    settings.setTerminalApplication(terminal);
	settings.save();

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
    dlg.setSaveNewApplications(true);
    if (dlg.exec() != QDialog::Accepted) {
        terminalCombo->setCurrentIndex(m_currentIndex);
        return;
    }
    const auto service = dlg.service();

    // if the selected service is already in the list
    const auto matching = terminalCombo->model()->match(terminalCombo->model()->index(0,0), Qt::DisplayRole, service->exec());
    if (!matching.isEmpty()) {
        const int index = matching.at(0).row();
        terminalCombo->setCurrentIndex(index);
        changed(index != m_currentIndex);
    } else {
        const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
        terminalCombo->insertItem(terminalCombo->count() -1, QIcon::fromTheme(icon), service->name(), service->exec());
        terminalCombo->setCurrentIndex(terminalCombo->count() - 2);

        changed(true);
    }

}
// vim: sw=4 ts=4 noet
