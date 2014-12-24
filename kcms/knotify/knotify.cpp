/*
    Copyright (C) 2000,2002 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (C) 2005,2006 Olivier Goffart <ogoffart at kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "knotify.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDBusInterface>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include <KApplication>
#include <KAboutData>
#include <KComboBox>
#include <KConfig>
#include <KConfigGroup>

#include <KNotifyConfigWidget>
#include <KPluginFactory>
#include <KPluginLoader>
#include <KStandardDirs>
#include <KUrlCompletion>
#include <KUrlRequester>
#include <KLocalizedString>
#include <KGlobal>
#include <QStandardPaths>

static const int COL_FILENAME = 1;

K_PLUGIN_FACTORY( NotifyFactory, registerPlugin<KCMKNotify>(); )
K_EXPORT_PLUGIN( NotifyFactory("kcmnotify") )

KCMKNotify::KCMKNotify(QWidget *parent, const QVariantList & )
    : KCModule(parent)
{
    setButtons( Help | Default | Apply );

    setQuickHelp( i18n("<h1>System Notifications</h1>"
                "KDE allows for a great deal of control over how you "
                "will be notified when certain events occur. There are "
                "several choices as to how you are notified:"
                "<ul><li>As the application was originally designed.</li>"
                "<li>With a beep or other noise.</li>"
                "<li>Via a popup dialog box with additional information.</li>"
                "<li>By recording the event in a logfile without "
                "any additional visual or audible alert.</li>"
                "</ul>"));

    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 0 );

    QLabel *label = new QLabel( i18n( "Event source:" ), this );
    m_appCombo = new KComboBox( false, this );
    m_appCombo->setSizeAdjustPolicy( QComboBox::AdjustToContents );
    m_appCombo->setObjectName( QLatin1String( "app combo" ) );

    // We want to sort the combo box
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(new QStandardItemModel(0, 1, proxyModel));
    // Now configure and set our sort model
    proxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_appCombo->setModel(proxyModel);

    QHBoxLayout *hbox = new QHBoxLayout();
    layout->addLayout( hbox );
    hbox->addWidget( label );
    hbox->addWidget( m_appCombo, 10 );

    m_notifyWidget = new KNotifyConfigWidget( this );
    layout->addWidget( m_notifyWidget );

    connect( m_notifyWidget, SIGNAL(changed(bool)), this,  SIGNAL(changed(bool)));

    m_appCombo->setFocus();

    connect( m_appCombo, SIGNAL( activated( int ) ),
             SLOT( slotAppActivated( int )) );

    KAboutData* ab = new KAboutData(
        "kcmknotify", i18n("KNotify"), "4.0",
        i18n("System Notification Control Panel Module"),
        KAboutLicense::GPL, i18n("(c) 2002-2006 KDE Team"));

    ab->addAuthor( i18n("Olivier Goffart"), QString(), "ogoffart@kde.org" );
    ab->addAuthor( i18n("Carsten Pfeiffer"), QString(), "pfeiffer@kde.org" );
    ab->addCredit( i18n("Charles Samuels"), i18n("Original implementation"),
        "charles@altair.dhs.org" );
    setAboutData( ab );
}

KCMKNotify::~KCMKNotify()
{
    KConfig _config("knotifyrc", KConfig::NoGlobals);
    KConfigGroup config(&_config, "Misc" );
    config.writeEntry( "LastConfiguredApp", m_appCombo->currentText() );
    config.sync();
}

void KCMKNotify::slotAppActivated(const int &index)
{
    QString text( m_appCombo->itemData(index).toString() );
    m_notifyWidget->save();
    m_notifyWidget->setApplication( text );
}

void KCMKNotify::defaults()
{
//    m_notifyWidget->resetDefaults( true ); // ask user
}
void KCMKNotify::load()
{
    //setEnabled( false );
    // setCursor( KCursor::waitCursor() );

    m_appCombo->clear();
//    m_notifyWidget->clear();

    QStringList fullpaths =
        KGlobal::dirs()->findAllResources("data", "*/*.notifyrc", KStandardDirs::NoDuplicates );

    foreach (const QString &fullPath, fullpaths )
    {
        int slash = fullPath.lastIndexOf( '/' );
        int dot = fullPath.lastIndexOf( '.' ) - 1;
        QString appname = slash < 0 ? QString() :  fullPath.mid( slash + 1, dot - slash);
        if ( !appname.isEmpty() )
        {
            KConfig config(fullPath, KConfig::NoGlobals, QStandardPaths::DataLocation);
            KConfigGroup globalConfig( &config, QString::fromLatin1("Global") );
            QString icon = globalConfig.readEntry(QString::fromLatin1("IconName"), QString::fromLatin1("misc"));
            QString description = globalConfig.readEntry( QString::fromLatin1("Comment"), appname );
            m_appCombo->addItem( QIcon::fromTheme( icon ), description, appname );
        }
    }

    m_appCombo->model()->sort(0);

    /*
    KConfig config( "knotifyrc", true, false );
    config.setGroup( "Misc" );
    QString appDesc = config.readEntry( "LastConfiguredApp", "KDE System Notifications" );

    if this code gets enabled again, make sure to apply r494965

    if ( !appDesc.isEmpty() )
        m_appCombo->setCurrentItem( appDesc );

     // sets the applicationEvents for KNotifyWidget
    slotAppActivated( m_appCombo->currentText() );

    // unsetCursor(); // unsetting doesn't work. sigh.
    setEnabled( true );
    emit changed( false );
    */

    if ( m_appCombo->count() > 0 ) {
        m_appCombo->setCurrentIndex(0);
        m_notifyWidget->setApplication( m_appCombo->itemData( 0 ).toString() );
    }

    emit changed(false);
}

void KCMKNotify::save()
{
    m_notifyWidget->save(); // will dcop knotify about its new config

    emit changed( true );
}

#include "knotify.moc"
