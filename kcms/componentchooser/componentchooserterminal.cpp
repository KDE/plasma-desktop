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

#include <QDBusConnection>
#include <QDBusMessage>
#include <QCheckBox>

#include <KMessageBox>
#include <KOpenWithDialog>
#include <KConfig>

#include <KUrlRequester>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KServiceTypeTrader>

#include <QUrl>

CfgTerminalEmulator::CfgTerminalEmulator(QWidget *parent)
    : CfgPlugin(parent)
{
    connect(this, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgTerminalEmulator::selectTerminalEmulator);
}

CfgTerminalEmulator::~CfgTerminalEmulator() {
}

void CfgTerminalEmulator::selectTerminalEmulator(int index)
{
    if (index == count() - 1) {
        selectTerminalApp();
    } else {
        emit changed(m_currentIndex != index);
    }
}

void CfgTerminalEmulator::load(KConfig *)
{
	TerminalSettings settings;
    const QString terminal = settings.terminalApplication();

    clear();
    m_currentIndex = -1;
    m_defaultIndex = -1;

    const auto constraint = QStringLiteral("'TerminalEmulator' in Categories AND (not exist NoDisplay OR NoDisplay == false)");
    const auto terminalEmulators = KServiceTypeTrader::self()->query(QStringLiteral("Application"), constraint);
    for (const auto &service : terminalEmulators) {
        addItem(QIcon::fromTheme(service->icon()), service->name(), service->exec());

        if (!terminal.isEmpty() && service->exec() == terminal) {
            setCurrentIndex(count() - 1);
            m_currentIndex = count() - 1;
        }
        if (service->exec() == QStringLiteral("konsole")) {
            m_defaultIndex = count() - 1;
        }
    }

    if (!terminal.isEmpty() && m_currentIndex == -1) {
        // we have a terminal specified by the user
        addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), terminal, terminal);
        setCurrentIndex(count() - 1);
        m_currentIndex = count() - 1;
    }

    // add a other option to add a new terminal emulator with KOpenWithDialog
    addItem(QIcon::fromTheme(QStringLiteral("application-x-shellscript")), i18n("Other..."), QStringLiteral());

    emit changed(false);
}

void CfgTerminalEmulator::save(KConfig *)
{
    if (currentIndex() == count() - 1) {
        // no terminal installed, nor selected
        return;
    }

    const QString terminal = currentData().toString();

    TerminalSettings settings;
    settings.setTerminalApplication(terminal);
    settings.save();

    m_currentIndex = currentIndex();

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
        setCurrentIndex(validLastCurrentIndex());
        return;
    }
    const auto service = dlg.service();

    // if the selected service is already in the list
    const auto matching = model()->match(model()->index(0,0), Qt::DisplayRole, service->exec());
    if (!matching.isEmpty()) {
        const int index = matching.at(0).row();
        setCurrentIndex(index);
        changed(index != m_currentIndex);
    } else {
        const QString icon = !service->icon().isEmpty() ? service->icon() : QStringLiteral("application-x-shellscript");
        insertItem(count() -1, QIcon::fromTheme(icon), service->name(), service->exec());
        setCurrentIndex(count() - 2);

        changed(true);
    }

}
// vim: sw=4 ts=4 noet
