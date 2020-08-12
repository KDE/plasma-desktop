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

#include "componentchooser.h"

#include "componentchooserbrowser.h"
#include "componentchooseremail.h"
#include "componentchooserfilemanager.h"
#ifdef Q_OS_UNIX
#include "componentchooserterminal.h"
#endif

#include <QDir>

#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>
#include <QLabel>
#include <KLocalizedString>
#include <KBuildSycocaProgressDialog>


ComponentChooser::ComponentChooser(QWidget *parent):
    QWidget(parent), Ui::ComponentChooser_UI()
{
	setupUi(this);

    const QString directory = QStandardPaths::locate(
                QStandardPaths::GenericDataLocation, QStringLiteral("kcm_componentchooser"), QStandardPaths::LocateDirectory);
    QStringList services;
    const QDir dir(directory);
    for (const auto &f: dir.entryList(QStringList("*.desktop"))) {
        services += dir.absoluteFilePath(f);
    }

    for (const QString &service : qAsConst(services))
	{
		KConfig cfg(service, KConfig::SimpleConfig);
        KConfigGroup cg = cfg.group(QByteArray());

        // fill the form layout
        const auto name = cg.readEntry("Name", i18n("Unknown"));
        CfgPlugin *loadedConfigWidget = loadConfigWidget(cfg.group(QByteArray()).readEntry("configurationType"));

        QLabel *label = new QLabel(i18nc("The label for the combobox: browser, terminal emulator...)", "%1:", name), this);
        label->setToolTip(cfg.group(QByteArray()).readEntry("Comment", QString()));

        formLayout->addRow(label, loadedConfigWidget);

        connect(loadedConfigWidget, &CfgPlugin::changed, this, &ComponentChooser::emitChanged);

        configWidgetMap.insert(service, loadedConfigWidget);
    }
}

CfgPlugin *ComponentChooser::loadConfigWidget(const QString &cfgType)
{
    CfgPlugin *loadedConfigWidget = nullptr;

    if (cfgType == QLatin1String("internal_email")) {
        loadedConfigWidget = new CfgEmailClient(this);
	}
#ifdef Q_OS_UNIX
    else if (cfgType == QLatin1String("internal_terminal")) {
        loadedConfigWidget = new CfgTerminalEmulator(this);
	}
#endif
    else if (cfgType == QLatin1String("internal_filemanager")) {
        loadedConfigWidget = new CfgFileManager(this);
    } else if (cfgType == QLatin1String("internal_browser")) {
        loadedConfigWidget = new CfgBrowser(this);
    } else {
        Q_ASSERT_X(false, "loadConfigWidget", "cfgType no supported");
    }
    loadedConfigWidget->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    loadedConfigWidget->setMinimumContentsLength(18);

    return loadedConfigWidget;
}

void ComponentChooser::emitChanged()
{
    bool somethingChanged = false;
    bool isDefaults = true;
    // check if another plugin has changed and default status
    for (CfgPlugin *plugin: qAsConst(configWidgetMap)) {
        somethingChanged |= plugin->hasChanged();
        isDefaults &= plugin->isDefaults();
    }

    emit changed(somethingChanged);
    emit defaulted(isDefaults);
}

ComponentChooser::~ComponentChooser()
{
    for (QWidget *configWidget : qAsConst(configWidgetMap)) {
        delete configWidget;
	}
}

void ComponentChooser::load() {
    for (auto it = configWidgetMap.constBegin(); it != configWidgetMap.constEnd(); ++it) {

        const auto service = it.key();
        const auto widget = it.value();

        CfgPlugin *plugin = dynamic_cast<CfgPlugin*>(widget);
        if (plugin) {
            KConfig cfg(service, KConfig::SimpleConfig);
            plugin->load( &cfg );
        }
	}
}

void ComponentChooser::save() {
    for (auto it = configWidgetMap.constBegin(); it != configWidgetMap.constEnd(); ++it) {

        const auto service = it.key();
        const auto widget = it.value();

        CfgPlugin *plugin = dynamic_cast<CfgPlugin*>(widget);
        if (plugin) {
            KConfig cfg(service, KConfig::SimpleConfig);
			plugin->save( &cfg );
		}
	}

    // refresh System configuration cache
    KBuildSycocaProgressDialog::rebuildKSycoca(this);
}

void ComponentChooser::restoreDefault() {
    for (CfgPlugin *plugin : qAsConst(configWidgetMap)) {
        plugin->defaults();
        emitChanged();
    }
}

// vim: sw=4 ts=4 noet
