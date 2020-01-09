/***************************************************************************
                          componentchooser.cpp  -  description
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

#include "componentchooser.h"

#include "componentchooserbrowser.h"
#include "componentchooseremail.h"
#include "componentchooserfilemanager.h"
#ifdef Q_OS_UNIX
#include "componentchooserterminal.h"
#endif

#include <QCheckBox>

#include <kmessagebox.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmimetypetrader.h>
#include <kurlrequester.h>
#include <ktoolinvocation.h>
#include <kconfiggroup.h>
#include <KServiceTypeTrader>
#include <KGlobal>
#include <QIcon>
#include <KLocalizedString>


//BEGIN  General kpart based Component selection

CfgComponent::CfgComponent(QWidget *parent)
    : QWidget(parent), Ui::ComponentConfig_UI(), CfgPlugin()
{
    setupUi( this );
    connect(ComponentSelector,SIGNAL(activated(const QString&)),this,SLOT(slotComponentChanged(const QString&)));
}

CfgComponent::~CfgComponent()
{
}

void CfgComponent::slotComponentChanged(const QString&) {
	emit changed(true);
}

void CfgComponent::save(KConfig *cfg) {
		// yes, this can happen if there are NO KTrader offers for this component
		if (!m_lookupDict.contains(ComponentSelector->currentText()))
			return;

		KConfigGroup mainGroup = cfg->group(QByteArray());
		QString serviceTypeToConfigure=mainGroup.readEntry("ServiceTypeToConfigure");
		KConfig store(mainGroup.readPathEntry("storeInFile", QStringLiteral("null")));
		KConfigGroup cg(&store, mainGroup.readEntry("valueSection"));
		cg.writePathEntry(mainGroup.readEntry("valueName", "kcm_componenchooser_null"),
                                  m_lookupDict.value(ComponentSelector->currentText()));
		store.sync();
		emit changed(false);
}

void CfgComponent::load(KConfig *cfg) {

	ComponentSelector->clear();
	m_lookupDict.clear();
	m_revLookupDict.clear();

	const KConfigGroup mainGroup = cfg->group(QByteArray());
	const QString serviceTypeToConfigure = mainGroup.readEntry("ServiceTypeToConfigure");

	const KService::List offers = KServiceTypeTrader::self()->query(serviceTypeToConfigure);

	for (const auto &service: offers) {
		ComponentSelector->addItem(service->name());
		m_lookupDict.insert(service->name(), service->desktopEntryName());
		m_revLookupDict.insert(service->desktopEntryName(), service->name());
	}

	KConfig store(mainGroup.readPathEntry("storeInFile",QStringLiteral("null")));
        const KConfigGroup group(&store, mainGroup.readEntry("valueSection"));
	QString setting = group.readEntry(mainGroup.readEntry("valueName","kcm_componenchooser_null"), QString());

	if (setting.isEmpty())
            setting = mainGroup.readEntry("defaultImplementation", QString());
	QString tmp = m_revLookupDict.value(setting);
	if (!tmp.isEmpty()) {
		for (int i=0;i<ComponentSelector->count();i++)
			if (tmp==ComponentSelector->itemText(i))
			{
				ComponentSelector->setCurrentIndex(i);
				break;
			}
        }
	emit changed(false);
}

void CfgComponent::defaults()
{
    //todo
}

bool CfgComponent::isDefaults() const
{
	return false;
}

//END  General kpart based Component selection






ComponentChooser::ComponentChooser(QWidget *parent):
    QWidget(parent), Ui::ComponentChooser_UI(), somethingChanged(false), configWidget(nullptr)
{
	setupUi(this);
	static_cast<QGridLayout*>(layout())->setRowStretch(1, 1);

	const QStringList directories = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("kcm_componentchooser"), QStandardPaths::LocateDirectory);
	QStringList services;
	for(const QString &directory : directories) {
		const QDir dir(directory);
		for(const QString &f: dir.entryList(QStringList("*.desktop"))) {
			services += dir.absoluteFilePath(f);
		}
	}

	for (const QString &service : qAsConst(services))
	{
		KConfig cfg(service, KConfig::SimpleConfig);
		KConfigGroup cg = cfg.group(QByteArray());
		QListWidgetItem *item = new QListWidgetItem(
			QIcon::fromTheme(cg.readEntry("Icon",QStringLiteral("preferences-desktop-default-applications"))), 
			cg.readEntry("Name",i18n("Unknown")));
		item->setData(Qt::UserRole, service);
		ServiceChooser->addItem(item);
		loadConfigWidget(service, cfg.group(QByteArray()).readEntry("configurationType"), item->text());
	}
	ServiceChooser->setFixedWidth(ServiceChooser->sizeHintForColumn(0) + 20);
	ServiceChooser->sortItems();
	connect(ServiceChooser,&QListWidget::currentItemChanged,this,&ComponentChooser::slotServiceSelected);
	ServiceChooser->setCurrentRow(0);
}

void ComponentChooser::loadConfigWidget(const QString &service, const QString &cfgType, const QString &name)
{
	QWidget *loadedConfigWidget = nullptr;
	if (cfgType.isEmpty() || (cfgType == QLatin1String("component")))
	{
		loadedConfigWidget = new CfgComponent(configContainer);
		static_cast<CfgComponent*>(loadedConfigWidget)->ChooserDocu->setText(i18n("Choose from the list below which component should be used by default for the %1 service.", name));
	}
	else if (cfgType==QLatin1String("internal_email"))
	{
		loadedConfigWidget = new CfgEmailClient(configContainer);
	}
#ifdef Q_OS_UNIX
	else if (cfgType==QLatin1String("internal_terminal"))
	{
		loadedConfigWidget = new CfgTerminalEmulator(configContainer);
	}
#endif
	else if (cfgType==QLatin1String("internal_filemanager"))
	{
		loadedConfigWidget = new CfgFileManager(configContainer);
	}
	else if (cfgType==QLatin1String("internal_browser"))
	{
		loadedConfigWidget = new CfgBrowser(configContainer);
	}

	if (loadedConfigWidget) {
		configWidgetMap.insert(service, loadedConfigWidget);
		configContainer->addWidget(loadedConfigWidget);
		connect(loadedConfigWidget, SIGNAL(changed(bool)), this, SLOT(emitChanged(bool)));
	}
}

void ComponentChooser::slotServiceSelected(QListWidgetItem* it) {
	if (!it) return;

	if (somethingChanged) {
		if (KMessageBox::questionYesNo(this,i18n("<qt>You changed the default component of your choice, do want to save that change now ?</qt>"),QString(),KStandardGuiItem::save(),KStandardGuiItem::discard())==KMessageBox::Yes) save();
	}
	const QString &service = it->data(Qt::UserRole).toString();
	KConfig cfg(service, KConfig::SimpleConfig);

	ComponentDescription->setText(cfg.group(QByteArray()).readEntry("Comment",i18n("No description available")));
	ComponentDescription->setMinimumSize(ComponentDescription->sizeHint());

	configWidget = configWidgetMap.value(service);
	if (configWidget) {
        configContainer->setCurrentWidget(configWidget);
        const auto plugin = dynamic_cast<CfgPlugin*>(configWidget);
        plugin->load(&cfg);
        emit defaulted(plugin->isDefaults());
	}

	emitChanged(false);
	latestEditedService = service;
}


void ComponentChooser::emitChanged(bool val) {
	somethingChanged=val;
	emit changed(val);

	CfgPlugin *plugin = dynamic_cast<CfgPlugin *>( configWidget );
	emit defaulted(plugin->isDefaults());
}


ComponentChooser::~ComponentChooser()
{
	for (QWidget *configWidget : configWidgetMap) {
		delete configWidget;
	}
}

void ComponentChooser::load() {
	if( configWidget )
	{
		CfgPlugin * plugin = dynamic_cast<CfgPlugin*>( configWidget );
		if( plugin )
		{
			KConfig cfg(latestEditedService, KConfig::SimpleConfig);
			plugin->load( &cfg );
		}
	}
}

void ComponentChooser::save() {
	if( configWidget )
	{
		CfgPlugin* plugin = dynamic_cast<CfgPlugin*>( configWidget );
		if( plugin )
		{
			KConfig cfg(latestEditedService, KConfig::SimpleConfig);
			plugin->save( &cfg );
		}
	}
}

void ComponentChooser::restoreDefault() {
    if (configWidget)
    {
        dynamic_cast<CfgPlugin*>(configWidget)->defaults();
        emitChanged(true);
    }

/*
	txtEMailClient->setText("kmail");
	chkRunTerminal->setChecked(false);

	// Check if -e is needed, I do not think so
	terminalLE->setText("xterm");  //No need for i18n
	terminalCB->setChecked(true);
	emitChanged(false);
*/
}

// vim: sw=4 ts=4 noet
