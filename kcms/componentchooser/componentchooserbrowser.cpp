/***************************************************************************
                          componentchooserbrowser.cpp
                             -------------------
    copyright            : (C) 2002 by Joseph Wenninger
    email                : jowenn@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundationi                            *
 *                                                                         *
 ***************************************************************************/

#include "componentchooserbrowser.h"
#include <kopenwithdialog.h>
#include <kglobalsettings.h>
#include <kconfiggroup.h>

#include <KLocalizedString>

#include "../migrationlib/kdelibs4config.h"

#include <KUrl>

CfgBrowser::CfgBrowser(QWidget *parent)
    : QWidget(parent), Ui::BrowserConfig_UI(),CfgPlugin()
{
    setupUi(this);
    connect(lineExec, &KLineEdit::textChanged, this, &CfgBrowser::configChanged);
    connect(radioKIO, &QRadioButton::toggled, this, &CfgBrowser::configChanged);
    connect(radioExec, &QRadioButton::toggled, this, &CfgBrowser::configChanged);
    connect(btnSelectBrowser, &QToolButton::clicked, this, &CfgBrowser::selectBrowser);
}

CfgBrowser::~CfgBrowser() {
}

void CfgBrowser::configChanged()
{
    emit changed(true);
}

void CfgBrowser::defaults()
{
    load(0);
}


void CfgBrowser::load(KConfig *) 
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), QLatin1String("General") );
    QString exec = config.readPathEntry( QLatin1String("BrowserApplication"), QString("") );
    if (exec.isEmpty())
    {
        radioKIO->setChecked(true);
        m_browserExec = exec;
        m_browserService = 0;
    }
    else
    {
        radioExec->setChecked(true);
        if (exec.startsWith('!'))
        {
            m_browserExec = exec.mid(1);
            m_browserService = 0;
        }
        else
        {
            m_browserService = KService::serviceByStorageId( exec );
            if (m_browserService)
                m_browserExec = m_browserService->desktopEntryName();
            else
                m_browserExec.clear();
        }
    }

    lineExec->setText(m_browserExec);

    emit changed(false);
}

void CfgBrowser::save(KConfig *)
{
    KConfigGroup config(KSharedConfig::openConfig("kdeglobals"), QLatin1String("General") );
    QString exec;
    if (radioExec->isChecked())
    {
        exec = lineExec->text();
        if (m_browserService && (exec == m_browserExec))
            exec = m_browserService->storageId(); // Use service
        else if (!exec.isEmpty())
            exec = '!' + exec; // Literal command
    }
    config.writePathEntry( QLatin1String("BrowserApplication"), exec); // KConfig::Normal|KConfig::Global
    config.sync();

    Kdelibs4SharedConfig::syncConfigGroup(&config, "kdeglobals");

    KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged);

    emit changed(false);
}

void CfgBrowser::selectBrowser()
{
    KUrl::List urlList;
    KOpenWithDialog dlg(urlList, i18n("Select preferred Web browser application:"), QString(), this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    m_browserService = dlg.service();
    if (m_browserService) {
        m_browserExec = m_browserService->desktopEntryName();
        if (m_browserExec.isEmpty())
            m_browserExec = m_browserService->exec();
    } else {
        m_browserExec = dlg.text();
    }
    lineExec->setText(m_browserExec);
}
