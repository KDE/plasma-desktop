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

#include <KBuildSycocaProgressDialog>
#include <KLocalizedString>
#include <KServiceTypeTrader>

#include "../migrationlib/kdelibs4config.h"

#include <QUrl>

CfgBrowser::CfgBrowser(QWidget *parent)
    : QWidget(parent), Ui::BrowserConfig_UI(),CfgPlugin()
{
    setupUi(this);
    connect(lineExec, &KLineEdit::textChanged, this, &CfgBrowser::configChanged);
    connect(radioKIO, &QRadioButton::toggled, this, &CfgBrowser::configChanged);
    connect(radioService, &QRadioButton::toggled, this, &CfgBrowser::selectBrowserApp);
    connect(browserCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), this, &CfgBrowser::selectBrowserApp);
    connect(radioExec, &QRadioButton::toggled, this, &CfgBrowser::configChanged);
    connect(btnSelectApplication, &QToolButton::clicked, this, &CfgBrowser::selectBrowser);
}

CfgBrowser::~CfgBrowser() {
}

void CfgBrowser::selectBrowserApp()
{
    const QString &storageId = browserCombo->currentData().toString();
    m_browserService = KService::serviceByStorageId(storageId);
    m_browserExec.clear();
    configChanged();
}

void CfgBrowser::configChanged()
{
    emit changed(true);
}

void CfgBrowser::defaults()
{
    load(nullptr);
}

void CfgBrowser::load(KConfig *) 
{
    const KConfigGroup config(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("General") );
    const QString exec = config.readPathEntry( QStringLiteral("BrowserApplication"), QString() );
    if (exec.isEmpty()) {
        radioKIO->setChecked(true);
        m_browserExec = exec;
        m_browserService = nullptr;
    } else {
        radioExec->setChecked(true);
        if (exec.startsWith('!')) {
            m_browserExec = exec.mid(1);
            m_browserService = nullptr;
        } else {
            m_browserService = KService::serviceByStorageId( exec );
            if (m_browserService) {
                m_browserExec = m_browserService->desktopEntryName();
            } else {
                m_browserExec.clear();
            }
        }
    }

    lineExec->setText(m_browserExec);

    browserCombo->clear();

    const auto &browsers = KServiceTypeTrader::self()->query(QStringLiteral("Application"),
                                                             QStringLiteral("'WebBrowser' in Categories"));
    for (const auto &service : browsers) {
        browserCombo->addItem(QIcon::fromTheme(service->icon()), service->name(), service->storageId());

        if ((m_browserService && m_browserService->storageId() == service->storageId()) || service->exec() == m_browserExec) {
            browserCombo->setCurrentIndex(browserCombo->count() - 1);
            radioService->setChecked(true);
        }
    }

    emit changed(false);
}

void CfgBrowser::save(KConfig *)
{
    KSharedConfig::Ptr profile = KSharedConfig::openConfig(QStringLiteral("kdeglobals"));
    KConfigGroup config(profile, QStringLiteral("General"));
    QString exec;
    if (radioService->isChecked()) {
        if (m_browserService) {
            exec = m_browserService->storageId();
        }
    } else if (radioExec->isChecked()) {
        exec = lineExec->text();
        if (m_browserService && (exec == m_browserExec)) {
            exec = m_browserService->storageId(); // Use service
        } else if (!exec.isEmpty()) {
            exec = '!' + exec; // Literal command
        }
    }
    config.writePathEntry( QStringLiteral("BrowserApplication"), exec); // KConfig::Normal|KConfig::Global
    config.sync();

    // Save the default browser as scheme handler for http(s) in mimeapps.list
    KSharedConfig::Ptr mimeAppList = KSharedConfig::openConfig(QStringLiteral("mimeapps.list"), KConfig::NoGlobals, QStandardPaths::GenericConfigLocation);
    if (mimeAppList->isConfigWritable(true /*warn user if not writable*/)) {
        KConfigGroup defaultApp(mimeAppList, "Default Applications");

        if (m_browserService) {
            defaultApp.writeXdgListEntry(QStringLiteral("x-scheme-handler/http"), QStringList(m_browserService->storageId()));
            defaultApp.writeXdgListEntry(QStringLiteral("x-scheme-handler/https"), QStringList(m_browserService->storageId()));
        } else {
            defaultApp.deleteEntry(QStringLiteral("x-scheme-handler/http"));
            defaultApp.deleteEntry(QStringLiteral("x-scheme-handler/https"));
        }
        mimeAppList->sync();

        KBuildSycocaProgressDialog::rebuildKSycoca(this);
    }

    Kdelibs4SharedConfig::syncConfigGroup(QLatin1String("General"), "kdeglobals");

    KGlobalSettings::self()->emitChange(KGlobalSettings::SettingsChanged);

    emit changed(false);
}

void CfgBrowser::selectBrowser()
{
    QList<QUrl> urlList;
    KOpenWithDialog dlg(urlList, i18n("Select preferred Web browser application:"), QString(), this);
    if (dlg.exec() != QDialog::Accepted)
        return;
    m_browserService = dlg.service();
    if (m_browserService) {
        // check if we have listed it in the browser combo, if so, put it there instead
        const int index = browserCombo->findData(m_browserService->storageId());
        if (index > -1) {
            browserCombo->setCurrentIndex(index);
            radioService->setChecked(true);
        } else {
            m_browserExec = m_browserService->desktopEntryName();
            if (m_browserExec.isEmpty()) {
                m_browserExec = m_browserService->exec();
            }
        }
    } else {
        m_browserExec = dlg.text();
    }
    lineExec->setText(m_browserExec);
}
