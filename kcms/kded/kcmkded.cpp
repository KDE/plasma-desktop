// vim: noexpandtab shiftwidth=4 tabstop=4
/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcmkded.h"

#include <QDBusInterface>
#include <QDBusReply>
#include <QGroupBox>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QLoggingCategory>
#include <QDialog>

#include <kaboutdata.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include <KConfigGroup>
#include <KPluginInfo>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KPluginMetaData>
#include <KLocalizedString>

K_PLUGIN_FACTORY(KDEDFactory,
        registerPlugin<KDEDConfig>();
        )
K_EXPORT_PLUGIN(KDEDFactory("kcmkded"))

Q_LOGGING_CATEGORY(KCM_KDED, "kcm_kded")


enum OnDemandColumns
{
	OnDemandService = 0,
	OnDemandStatus = 1,
	OnDemandDescription = 2
};

enum StartupColumns
{
	StartupUse = 0,
	StartupService = 1,
	StartupStatus = 2,
	StartupDescription = 3
};



static const int LibraryRole = Qt::UserRole + 1;

KDEDConfig::KDEDConfig(QWidget* parent, const QVariantList &) :
	KCModule( parent )
{
	KAboutData *about =
        new KAboutData( I18N_NOOP( "kcmkded" ), i18n( "KDE Service Manager" ),
                QStringLiteral("1.0"), QString(), KAboutLicense::GPL,
				i18n( "(c) 2002 Daniel Molkentin" ) );
	about->addAuthor(i18n("Daniel Molkentin"), QString(),QStringLiteral("molkentin@kde.org"));
	setAboutData( about );
	setButtons(Apply|Default|Help);
	setQuickHelp( i18n("<h1>Service Manager</h1><p>This module allows you to have an overview of all plugins of the "
			"KDE Daemon, also referred to as KDE Services. Generally, there are two types of service:</p>"
			"<ul><li>Services invoked at startup</li><li>Services called on demand</li></ul>"
			"<p>The latter are only listed for convenience. The startup services can be started and stopped. "
			"In Administrator mode, you can also define whether services should be loaded at startup.</p>"
			"<p><b> Use this with care: some services are vital for Plasma; do not deactivate services if you"
			" do not know what you are doing.</b></p>"));

	RUNNING = i18n("Running")+' ';
	NOT_RUNNING = i18n("Not running")+' ';

	QVBoxLayout *lay = new QVBoxLayout( this );
	lay->setMargin( 0 );

	QGroupBox *gb = new QGroupBox( i18n("Load-on-Demand Services"), this );
	gb->setWhatsThis( i18n("This is a list of available KDE services which will "
			"be started on demand. They are only listed for convenience, as you "
			"cannot manipulate these services."));
	lay->addWidget( gb );

	QVBoxLayout *gblay = new QVBoxLayout( gb );

	_lvLoD = new QTreeWidget( gb );
	QStringList cols;
	cols.append( i18n("Service") );
	cols.append( i18n("Status") );
	cols.append( i18n("Description") );
	_lvLoD->setHeaderLabels( cols );
	_lvLoD->setAllColumnsShowFocus(true);
	_lvLoD->setRootIsDecorated( false );
	_lvLoD->setSortingEnabled(true);
	_lvLoD->sortByColumn(OnDemandService, Qt::AscendingOrder);
	_lvLoD->header()->setStretchLastSection(true);
	gblay->addWidget( _lvLoD );

 	gb = new QGroupBox( i18n("Startup Services"), this );
	gb->setWhatsThis( i18n("This shows all KDE services that can be loaded "
				"on Plasma startup. Checked services will be invoked on next startup. "
				"Be careful with deactivation of unknown services."));
	lay->addWidget( gb );

	gblay = new QVBoxLayout( gb );

	_lvStartup = new QTreeWidget( gb );
	cols.clear();
	cols.append( i18n("Use") );
	cols.append( i18n("Service") );
	cols.append( i18n("Status") );
	cols.append( i18n("Description") );
	_lvStartup->setHeaderLabels( cols );
	_lvStartup->setAllColumnsShowFocus(true);
	_lvStartup->setRootIsDecorated( false );
	_lvStartup->setSortingEnabled(true);
	_lvStartup->sortByColumn(StartupService, Qt::AscendingOrder);
	_lvStartup->header()->setStretchLastSection(true);
	gblay->addWidget( _lvStartup );

	QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal, gb);
	_pbStart = buttonBox->addButton( i18n("Start") , QDialogButtonBox::ActionRole  );
	_pbStop = buttonBox->addButton( i18n("Stop") , QDialogButtonBox::ActionRole );
	gblay->addWidget( buttonBox );

	_pbStart->setEnabled( false );
	_pbStop->setEnabled( false );

	connect(_pbStart, &QPushButton::clicked, this, &KDEDConfig::slotStartService);
	connect(_pbStop, &QPushButton::clicked, this, &KDEDConfig::slotStopService);
	connect(_lvLoD, &QTreeWidget::itemSelectionChanged, this, &KDEDConfig::slotLodItemSelected);
	connect(_lvStartup, &QTreeWidget::itemSelectionChanged, this, &KDEDConfig::slotStartupItemSelected);
	connect(_lvStartup, &QTreeWidget::itemChanged, this, &KDEDConfig::slotItemChecked);

}

QString setModuleGroup(const KPluginMetaData &module)
{
	return QStringLiteral("Module-%1").arg(module.pluginId());
}

bool KDEDConfig::autoloadEnabled(KConfig *config, const KPluginMetaData &module)
{
	KConfigGroup cg(config, setModuleGroup(module));
	return cg.readEntry("autoload", true);
}

void KDEDConfig::setAutoloadEnabled(KConfig *config, const KPluginMetaData &module, bool b)
{
	KConfigGroup cg(config, setModuleGroup(module));
	return cg.writeEntry("autoload", b);
}

// This code was copied from kded.cpp
// TODO: move this KCM to the KDED framework and share the code?
static QVector<KPluginMetaData> availableModules()
{
	QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("kf5/kded"));
	QSet<QString> moduleIds;
	foreach (const KPluginMetaData &md, plugins) {
		moduleIds.insert(md.pluginId());
	}
	// also search for old .desktop based kded modules
	KPluginInfo::List oldStylePlugins = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QStringLiteral("KDEDModule")));
	foreach (const KPluginInfo &info, oldStylePlugins) {
		if (moduleIds.contains(info.pluginName())) {
			qCWarning(KCM_KDED).nospace() << "kded module " << info.pluginName() << " has already been found using "
			"JSON metadata, please don't install the now unneeded .desktop file (" << info.entryPath() << ").";
		} else {
			qCDebug(KCM_KDED).nospace() << "kded module " << info.pluginName() << " still uses .desktop files ("
			<< info.entryPath() << "). Please port it to JSON metadata.";
			plugins.append(info.toMetaData());
		}
	}
	return plugins;
}

// this code was copied from kded.cpp
static bool isModuleLoadedOnDemand(const KPluginMetaData &module)
{
	bool loadOnDemand = true;
	// use toVariant() since it could be string or bool in the json and QJsonObject does not convert
	QVariant p = module.rawData().value(QStringLiteral("X-KDE-Kded-load-on-demand")).toVariant();
	if (p.isValid() && p.canConvert<bool>() && (p.toBool() == false)) {
		loadOnDemand = false;
	}
	return loadOnDemand;
}

void KDEDConfig::load()
{
	KConfig kdedrc(QStringLiteral("kded5rc"), KConfig::NoGlobals);

	_lvStartup->clear();
	_lvLoD->clear();

	QTreeWidgetItem* treeitem = nullptr;
	const auto modules = availableModules();
	for (const KPluginMetaData &mod : modules) {
		QString servicePath = mod.metaDataFileName();

		// autoload defaults to false if it is not found
		const bool autoload = mod.rawData().value(QStringLiteral("X-KDE-Kded-autoload")).toVariant().toBool();
		// keep estimating dbusModuleName in sync with KDEDModule (kdbusaddons) and kded (kded)
		// currently (KF5) the module name in the D-Bus object path is set by the pluginId
		const QString dbusModuleName = mod.pluginId();
		qCDebug(KCM_KDED) << "reading kded info from" << servicePath << "autoload =" << autoload << "dbus module name =" << dbusModuleName;

		// The logic has to be identical to Kded::initModules.
		// They interpret X-KDE-Kded-autoload as false if not specified
		//                X-KDE-Kded-load-on-demand as true if not specified
		if (autoload) {
			treeitem = new QTreeWidgetItem();
			treeitem->setCheckState(StartupUse, autoloadEnabled(&kdedrc, mod) ? Qt::Checked : Qt::Unchecked);
			treeitem->setText(StartupService, mod.name());
			treeitem->setText(StartupDescription, mod.description());
			treeitem->setText(StartupStatus, NOT_RUNNING);
			treeitem->setData(StartupService, LibraryRole, dbusModuleName);
			_lvStartup->addTopLevelItem(treeitem);
		}
		else if (isModuleLoadedOnDemand(mod)) {
			treeitem = new QTreeWidgetItem();
			treeitem->setText(OnDemandService, mod.name() );
			treeitem->setText(OnDemandDescription, mod.description());
			treeitem->setText(OnDemandStatus, NOT_RUNNING);
			treeitem->setData(OnDemandService, LibraryRole, dbusModuleName);
			_lvLoD->addTopLevelItem(treeitem);
		}
		else {
			qCWarning(KCM_KDED) << "kcmkded: Module " << mod.name() << "from file" << mod.metaDataFileName() << " not loaded on demand or startup! Skipping.";
		}
	}

	_lvStartup->resizeColumnToContents(StartupUse);
	_lvStartup->resizeColumnToContents(StartupService);
	_lvStartup->resizeColumnToContents(StartupStatus);

	_lvLoD->resizeColumnToContents(OnDemandService);
	_lvLoD->resizeColumnToContents(OnDemandStatus);

	getServiceStatus();

	emit changed(false);
}

void KDEDConfig::save()
{
	KConfig kdedrc(QStringLiteral("kded5rc"), KConfig::NoGlobals);

	const auto modules = availableModules();
	for (const KPluginMetaData &mod : modules) {
		qCDebug(KCM_KDED) << "saving settings for kded module" << mod.pluginId();
		// autoload defaults to false if it is not found
		const bool autoload = mod.rawData().value(QStringLiteral("X-KDE-Kded-autoload")).toVariant().toBool();
		if (autoload) {
			const QString libraryName = mod.pluginId();
			int count = _lvStartup->topLevelItemCount();
			for(int i = 0; i < count; ++i) {
				QTreeWidgetItem *treeitem = _lvStartup->topLevelItem(i);
				if ( treeitem->data(StartupService, LibraryRole ).toString() == libraryName) {
					// we found a match, now compare and see what changed
					setAutoloadEnabled(&kdedrc, mod, treeitem->checkState( StartupUse ) == Qt::Checked);
					break;
				}
			}
		}
	}
	kdedrc.sync();

	emit changed(false);

	QDBusInterface kdedInterface(QStringLiteral("org.kde.kded5"), QStringLiteral("/kded"), QStringLiteral("org.kde.kded5"));
	kdedInterface.call(QStringLiteral("reconfigure"));
	QTimer::singleShot(0, this, &KDEDConfig::slotServiceRunningToggled);
}


void KDEDConfig::defaults()
{
	int count = _lvStartup->topLevelItemCount();
	for( int i = 0; i < count; ++i )
	{
		_lvStartup->topLevelItem( i )->setCheckState( StartupUse, Qt::Checked );
	}

	getServiceStatus();

	emit changed(true);
}


void KDEDConfig::getServiceStatus()
{
	QStringList modules;
	QDBusInterface kdedInterface( QStringLiteral("org.kde.kded5"), QStringLiteral("/kded"), QStringLiteral("org.kde.kded5") );
	QDBusReply<QStringList> reply = kdedInterface.call( QStringLiteral("loadedModules")  );

	if ( reply.isValid() ) {
		modules = reply.value();
	}
	else {
		_lvLoD->setEnabled( false );
		_lvStartup->setEnabled( false );
		KMessageBox::error(this, i18n("Unable to contact KDED."));
		return;
	}
	qCDebug(KCM_KDED) << "Loaded kded modules:" << modules;

    // Initialize
	int count = _lvLoD->topLevelItemCount();
	for( int i = 0; i < count; ++i )
                _lvLoD->topLevelItem( i )->setText( OnDemandStatus, NOT_RUNNING );
	count = _lvStartup->topLevelItemCount();
	for( int i = 0; i < count; ++i )
                _lvStartup->topLevelItem( i )->setText( StartupStatus, NOT_RUNNING );

    // Fill
	foreach( const QString& module, modules )
	{
		bool found = false;

		count = _lvLoD->topLevelItemCount();
		for( int i = 0; i < count; ++i )
		{
			QTreeWidgetItem *treeitem = _lvLoD->topLevelItem( i );
			if ( treeitem->data( OnDemandService, LibraryRole ).toString() == module )
			{
				treeitem->setText( OnDemandStatus, RUNNING );
				found = true;
				break;
			}
		}

		count = _lvStartup->topLevelItemCount();
		for( int i = 0; i < count; ++i )
		{
			QTreeWidgetItem *treeitem = _lvStartup->topLevelItem( i );
			if ( treeitem->data( StartupService, LibraryRole ).toString() == module )
			{
				treeitem->setText( StartupStatus, RUNNING );
				found = true;
				break;
			}
		}

		if (!found)
		{
			qCDebug(KCM_KDED) << "Could not relate module " << module;
#ifndef NDEBUG
			qCDebug(KCM_KDED) << "Candidates were:";
			count = _lvLoD->topLevelItemCount();
			for( int i = 0; i < count; ++i )
			{
				QTreeWidgetItem *treeitem = _lvLoD->topLevelItem( i );
				qCDebug(KCM_KDED) << treeitem->data( OnDemandService, LibraryRole ).toString();
			}

			count = _lvStartup->topLevelItemCount();
			for( int i = 0; i < count; ++i )
			{
				QTreeWidgetItem *treeitem = _lvStartup->topLevelItem( i );
				qCDebug(KCM_KDED) << treeitem->data( StartupService, LibraryRole ).toString();
			}
#endif
		}

	}

}

void KDEDConfig::slotReload()
{
	QString current;
	if ( !_lvStartup->selectedItems().isEmpty() )
		current = _lvStartup->selectedItems().at(0)->data( StartupService, LibraryRole ).toString();
	load();
	if ( !current.isEmpty() )
	{
		int count = _lvStartup->topLevelItemCount();
		for( int i = 0; i < count; ++i )
		{
			QTreeWidgetItem *treeitem = _lvStartup->topLevelItem( i );
			if ( treeitem->data( StartupService, LibraryRole ).toString() == current )
			{
				_lvStartup->setCurrentItem( treeitem, 0, QItemSelectionModel::ClearAndSelect );
				break;
			}
		}
	}
}

void KDEDConfig::slotStartupItemSelected()
{
	if ( _lvStartup->selectedItems().isEmpty() ) {
		// Disable the buttons
		_pbStart->setEnabled( false );
		_pbStop->setEnabled( false );
		return;
	}

	// Deselect a currently selected element in the "load on demand" treeview
	_lvLoD->setCurrentItem(nullptr, 0, QItemSelectionModel::Clear);

	QTreeWidgetItem *item = _lvStartup->selectedItems().at(0);
	if ( item->text(StartupStatus) == RUNNING ) {
		_pbStart->setEnabled( false );
		_pbStop->setEnabled( true );
	}
	else if ( item->text(StartupStatus) == NOT_RUNNING ) {
		_pbStart->setEnabled( true );
		_pbStop->setEnabled( false );
	}
	else // Error handling, better do nothing
	{
		_pbStart->setEnabled( false );
		_pbStop->setEnabled( false );
	}

	getServiceStatus();
}

void KDEDConfig::slotLodItemSelected()
{
	if ( _lvLoD->selectedItems().isEmpty() )
		return;

	// Deselect a currently selected element in the "load on startup" treeview
	_lvStartup->setCurrentItem(nullptr, 0, QItemSelectionModel::Clear);
}

void KDEDConfig::slotServiceRunningToggled()
{
	getServiceStatus();
	slotStartupItemSelected();
}

void KDEDConfig::slotStartService()
{
	QString service = _lvStartup->selectedItems().at(0)->data( StartupService, LibraryRole ).toString();

	QDBusInterface kdedInterface( QStringLiteral("org.kde.kded5"), QStringLiteral("/kded"),QStringLiteral("org.kde.kded5") );
	QDBusReply<bool> reply = kdedInterface.call( QStringLiteral("loadModule"), service  );

	if ( reply.isValid() ) {
		if ( reply.value() )
			slotServiceRunningToggled();
		else
			KMessageBox::error(this, "<qt>" + i18n("Unable to start server <em>%1</em>.", service) + "</qt>");
	}
	else {
		KMessageBox::error(this, "<qt>" + i18n("Unable to start service <em>%1</em>.<br /><br /><i>Error: %2</i>",
											service, reply.error().message()) + "</qt>" );
	}
}

void KDEDConfig::slotStopService()
{
	QString service = _lvStartup->selectedItems().at(0)->data( StartupService, LibraryRole ).toString();
	qCDebug(KCM_KDED) << "Stopping: " << service;

	QDBusInterface kdedInterface( QStringLiteral("org.kde.kded5"), QStringLiteral("/kded"), QStringLiteral("org.kde.kded5") );
	QDBusReply<bool> reply = kdedInterface.call( QStringLiteral("unloadModule"), service  );

	if ( reply.isValid() ) {
		if ( reply.value() )
			slotServiceRunningToggled();
		else
			KMessageBox::error(this, "<qt>" + i18n("Unable to stop service <em>%1</em>.", service) + "</qt>");
	}
	else {
		KMessageBox::error(this, "<qt>" + i18n("Unable to stop service <em>%1</em>.<br /><br /><i>Error: %2</i>",
											service, reply.error().message()) + "</qt>" );
	}
}

void KDEDConfig::slotItemChecked(QTreeWidgetItem*, int column)
{
	// We only listen to changes the user did.
	if (column==StartupUse) {
		emit changed(true);
	}
}

#include "kcmkded.moc"
