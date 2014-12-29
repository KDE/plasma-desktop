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

#include <QtDBus/QtDBus>
#include <QGroupBox>
#include <QPushButton>
#include <QTimer>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QDialogButtonBox>
#include <QLoggingCategory>
#include <QDialog>

#include <KConfigGroup>

#include <kaboutdata.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kservicetypetrader.h>

#include <KPluginFactory>
#include <KPluginLoader>
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
	about->addAuthor(i18n("Daniel Molkentin"), QString(),"molkentin@kde.org");
	setAboutData( about );
	setButtons(Apply|Default|Help);
	setQuickHelp( i18n("<h1>Service Manager</h1><p>This module allows you to have an overview of all plugins of the "
			"KDE Daemon, also referred to as KDE Services. Generally, there are two types of service:</p>"
			"<ul><li>Services invoked at startup</li><li>Services called on demand</li></ul>"
			"<p>The latter are only listed for convenience. The startup services can be started and stopped. "
			"In Administrator mode, you can also define whether services should be loaded at startup.</p>"
			"<p><b> Use this with care: some services are vital for KDE; do not deactivate services if you"
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
				"on KDE startup. Checked services will be invoked on next startup. "
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

	connect(_pbStart, SIGNAL(clicked()), SLOT(slotStartService()));
	connect(_pbStop, SIGNAL(clicked()), SLOT(slotStopService()));
	connect(_lvLoD, SIGNAL(itemSelectionChanged()), SLOT(slotLodItemSelected()));
	connect(_lvStartup, SIGNAL(itemSelectionChanged()), SLOT(slotStartupItemSelected()));
	connect(_lvStartup, SIGNAL(itemChanged(QTreeWidgetItem*, int)), SLOT(slotItemChecked(QTreeWidgetItem*, int)) );

}

QString setModuleGroup(const QString &filename)
{
	QString module = filename;
	int i = module.lastIndexOf('/');
	if (i != -1)
	   module = module.mid(i+1);
	i = module.lastIndexOf('.');
	if (i != -1)
	   module = module.left(i);

	return QString("Module-%1").arg(module);
}

bool KDEDConfig::autoloadEnabled(KConfig *config, const QString &filename)
{
	KConfigGroup cg(config, setModuleGroup(filename));
	return cg.readEntry("autoload", true);
}

void KDEDConfig::setAutoloadEnabled(KConfig *config, const QString &filename, bool b)
{
	KConfigGroup cg(config, setModuleGroup(filename));
	return cg.writeEntry("autoload", b);
}

void KDEDConfig::load()
{
	KConfig kdedrc( "kded5rc", KConfig::NoGlobals );

	_lvStartup->clear();
	_lvLoD->clear();

	KService::List offers = KServiceTypeTrader::self()->query( "KDEDModule" );
	QTreeWidgetItem* treeitem = 0L;
	for ( KService::List::const_iterator it = offers.constBegin();
	      it != offers.constEnd(); ++it)
	{
		QString servicePath = (*it)->entryPath();
		qCDebug(KCM_KDED) << servicePath;

        const KDesktopFile file(QStandardPaths::GenericDataLocation, "kservices5/" + servicePath );
		const KConfigGroup grp = file.desktopGroup();
		// The logic has to be identical to Kded::initModules.
		// They interpret X-KDE-Kded-autoload as false if not specified
		//                X-KDE-Kded-load-on-demand as true if not specified
		if ( grp.readEntry("X-KDE-Kded-autoload", false) ) {
			treeitem = new QTreeWidgetItem();
			treeitem->setCheckState( StartupUse, autoloadEnabled(&kdedrc, file.name()) ? Qt::Checked : Qt::Unchecked );
			treeitem->setText( StartupService, file.readName() );
			treeitem->setText( StartupDescription, file.readComment() );
			treeitem->setText( StartupStatus, NOT_RUNNING );
			if (grp.hasKey("X-KDE-DBus-ModuleName")) {
				treeitem->setData( StartupService, LibraryRole, grp.readEntry("X-KDE-DBus-ModuleName") );
			} else {
				qCWarning(KCM_KDED) << "X-KDE-DBUS-ModuleName not set for module " << file.readName();
				treeitem->setData( StartupService, LibraryRole, grp.readEntry("X-KDE-Library") );
			}
			_lvStartup->addTopLevelItem( treeitem );
		}
		else if ( grp.readEntry("X-KDE-Kded-load-on-demand", true) ) {
			treeitem = new QTreeWidgetItem();
			treeitem->setText( OnDemandService, file.readName() );
			treeitem->setText( OnDemandDescription, file.readComment() );
			treeitem->setText( OnDemandStatus, NOT_RUNNING );
			if (grp.hasKey("X-KDE-DBus-ModuleName")) {
				treeitem->setData( OnDemandService, LibraryRole, grp.readEntry("X-KDE-DBus-ModuleName") );
			} else {
				qCWarning(KCM_KDED) << "X-KDE-DBUS-ModuleName not set for module " << file.readName();
				treeitem->setData( OnDemandService, LibraryRole, grp.readEntry("X-KDE-Library") );
			}
			_lvLoD->addTopLevelItem( treeitem );
		}
		else {
			qCWarning(KCM_KDED) << "kcmkded: Module " << file.readName() << " not loaded on demand or startup! Skipping.";
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
	KConfig kdedrc("kded5rc", KConfig::NoGlobals);

	KService::List offers = KServiceTypeTrader::self()->query( "KDEDModule" );
	for ( KService::List::const_iterator it = offers.constBegin();
	      it != offers.constEnd(); ++it)
	{
		QString servicePath = (*it)->entryPath();
		qCDebug(KCM_KDED) << servicePath;

        const KDesktopFile file(QStandardPaths::GenericDataLocation, "kservices5/" + servicePath );
		const KConfigGroup grp = file.desktopGroup();
		if (grp.readEntry("X-KDE-Kded-autoload", false)){

			QString libraryName = grp.readEntry( "X-KDE-Library" );
			int count = _lvStartup->topLevelItemCount();
			for( int i = 0; i < count; ++i )
			{
				QTreeWidgetItem *treeitem = _lvStartup->topLevelItem( i );
				if ( treeitem->data( StartupService, LibraryRole ).toString() == libraryName )
				{
					// we found a match, now compare and see what changed
					setAutoloadEnabled( &kdedrc, servicePath, treeitem->checkState( StartupUse ) == Qt::Checked);
					break;
				}
			}
		}
	}
	kdedrc.sync();

	emit changed(false);

	QDBusInterface kdedInterface( "org.kde.kded5", "/kded", "org.kde.kded5" );
	kdedInterface.call( "reconfigure" );
	QTimer::singleShot(0, this, SLOT(slotServiceRunningToggled()));
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
	QDBusInterface kdedInterface( "org.kde.kded5", "/kded", "org.kde.kded5" );
	QDBusReply<QStringList> reply = kdedInterface.call( "loadedModules"  );

	if ( reply.isValid() ) {
		modules = reply.value();
	}
	else {
		_lvLoD->setEnabled( false );
		_lvStartup->setEnabled( false );
		KMessageBox::error(this, i18n("Unable to contact KDED."));
		return;
	}

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
	_lvLoD->setCurrentItem(NULL, 0, QItemSelectionModel::Clear);

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
	_lvStartup->setCurrentItem(NULL, 0, QItemSelectionModel::Clear);
}

void KDEDConfig::slotServiceRunningToggled()
{
	getServiceStatus();
	slotStartupItemSelected();
}

void KDEDConfig::slotStartService()
{
	QString service = _lvStartup->selectedItems().at(0)->data( StartupService, LibraryRole ).toString();

	QDBusInterface kdedInterface( "org.kde.kded5", "/kded","org.kde.kded5" );
	QDBusReply<bool> reply = kdedInterface.call( "loadModule", service  );

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

	QDBusInterface kdedInterface( "org.kde.kded5", "/kded", "org.kde.kded5" );
	QDBusReply<bool> reply = kdedInterface.call( "unloadModule", service  );

	if ( reply.isValid() ) {
		if ( reply.value() )
			slotServiceRunningToggled();
		else
			KMessageBox::error(this, "<qt>" + i18n("Unable to stop server <em>%1</em>.", service) + "</qt>");
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
