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

#include "componentchooser.h"

#include "componentchooserbrowser.h"
#include "componentchooseremail.h"
#include "componentchooserfilemanager.h"
#ifdef Q_OS_UNIX
#include "componentchooserterminal.h"
#endif
#include <config-X11.h>
#if HAVE_X11
#include "componentchooserwm.h"
#endif

#include <QCheckBox>

#include <klocale.h>
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
		KConfig store(mainGroup.readPathEntry("storeInFile", "null"));
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

	for (KService::List::const_iterator tit = offers.begin(); tit != offers.end(); ++tit) {
		ComponentSelector->addItem((*tit)->name());
		m_lookupDict.insert((*tit)->name(), (*tit)->desktopEntryName());
		m_revLookupDict.insert((*tit)->desktopEntryName(), (*tit)->name());
	}

	KConfig store(mainGroup.readPathEntry("storeInFile","null"));
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

//END  General kpart based Component selection






ComponentChooser::ComponentChooser(QWidget *parent):
    QWidget(parent), Ui::ComponentChooser_UI(), somethingChanged(false), configWidget(0)
{
	setupUi(this);
	static_cast<QGridLayout*>(layout())->setRowStretch(1, 1);

	const QStringList services=KGlobal::dirs()->findAllResources( "data","kcm_componentchooser/*.desktop",
															KStandardDirs::NoDuplicates);
	for (QStringList::const_iterator it=services.constBegin(); it!=services.constEnd(); ++it)
	{
		KConfig cfg(*it, KConfig::SimpleConfig);
		KConfigGroup cg = cfg.group(QByteArray());
		QListWidgetItem *item = new QListWidgetItem(
			QIcon::fromTheme(cg.readEntry("Icon",QString("preferences-desktop-default-applications"))), 
			cg.readEntry("Name",i18n("Unknown")));
		item->setData(Qt::UserRole, (*it));
		ServiceChooser->addItem(item);
	}
	ServiceChooser->setFixedWidth(ServiceChooser->sizeHintForColumn(0) + 20);
	ServiceChooser->sortItems();
	connect(ServiceChooser,SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),this,SLOT(slotServiceSelected(QListWidgetItem*)));
	ServiceChooser->setCurrentRow(0);
	slotServiceSelected(ServiceChooser->item(0));

}

void ComponentChooser::slotServiceSelected(QListWidgetItem* it) {
	if (!it) return;

	if (somethingChanged) {
		if (KMessageBox::questionYesNo(this,i18n("<qt>You changed the default component of your choice, do want to save that change now ?</qt>"),QString(),KStandardGuiItem::save(),KStandardGuiItem::discard())==KMessageBox::Yes) save();
	}
	KConfig cfg(it->data(Qt::UserRole).toString(), KConfig::SimpleConfig);

	ComponentDescription->setText(cfg.group(QByteArray()).readEntry("Comment",i18n("No description available")));
	ComponentDescription->setMinimumSize(ComponentDescription->sizeHint());


	QString cfgType=cfg.group(QByteArray()).readEntry("configurationType");
	QWidget *newConfigWidget = 0;
	if (cfgType.isEmpty() || (cfgType=="component"))
	{
		if (!(configWidget && qobject_cast<CfgComponent*>(configWidget)))
		{
			CfgComponent* cfgcomp = new CfgComponent(configContainer);
                        cfgcomp->ChooserDocu->setText(i18n("Choose from the list below which component should be used by default for the %1 service.", it->text()));
			newConfigWidget = cfgcomp;
		}
                else
                {
                        static_cast<CfgComponent*>(configWidget)->ChooserDocu->setText(i18n("Choose from the list below which component should be used by default for the %1 service.", it->text()));
                }
	}
	else if (cfgType=="internal_email")
	{
		if (!(configWidget && qobject_cast<CfgEmailClient*>(configWidget)))
		{
			newConfigWidget = new CfgEmailClient(configContainer);
		}

	}
#ifdef Q_OS_UNIX
	else if (cfgType=="internal_terminal")
	{
		if (!(configWidget && qobject_cast<CfgTerminalEmulator*>(configWidget)))
		{
			newConfigWidget = new CfgTerminalEmulator(configContainer);
		}

	}
#if HAVE_X11
	else if (cfgType=="internal_wm")
	{
		if (!(configWidget && qobject_cast<CfgWm*>(configWidget)))
		{
			newConfigWidget = new CfgWm(configContainer);
		}

	}
#endif
#endif
	else if (cfgType=="internal_filemanager")
	{
		if (!(configWidget && qobject_cast<CfgFileManager*>(configWidget)))
		{
			newConfigWidget = new CfgFileManager(configContainer);
		}

	}
	else if (cfgType=="internal_browser")
	{
		if (!(configWidget && qobject_cast<CfgBrowser*>(configWidget)))
		{
			newConfigWidget = new CfgBrowser(configContainer);
		}

	}

	if (newConfigWidget)
	{
		configContainer->addWidget(newConfigWidget);
		configContainer->setCurrentWidget (newConfigWidget);
		configContainer->removeWidget(configWidget);
		delete configWidget;
		configWidget=newConfigWidget;
		connect(configWidget,SIGNAL(changed(bool)),this,SLOT(emitChanged(bool)));
	        configContainer->setMinimumSize(configWidget->sizeHint());
	}

	if (configWidget)
		dynamic_cast<CfgPlugin*>(configWidget)->load(&cfg);

        emitChanged(false);
	latestEditedService=it->data(Qt::UserRole).toString();
}


void ComponentChooser::emitChanged(bool val) {
	somethingChanged=val;
	emit changed(val);
}


ComponentChooser::~ComponentChooser()
{
	delete configWidget;
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
