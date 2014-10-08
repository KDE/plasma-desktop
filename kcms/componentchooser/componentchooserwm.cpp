/***************************************************************************
                          componentchooserwm.cpp  -  description
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

#include <config-workspace.h>

#include "componentchooserwm.h"

#include <kglobal.h>
#include <kdebug.h>
#include <kdesktopfile.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <ktimerdialog.h>
#include <qdbusinterface.h>
#include <qdbusconnectioninterface.h>
#include <netwm.h>
#include <qx11info_x11.h>

CfgWm::CfgWm(QWidget *parent)
: QWidget(parent)
, Ui::WmConfig_UI()
, CfgPlugin()
, wmLaunchingState( WmNone )
, wmProcess( NULL )
{
    setupUi(this);
    connect(wmCombo,SIGNAL(activated(int)), this, SLOT(configChanged()));
    connect(kwinRB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(differentRB,SIGNAL(toggled(bool)),this,SLOT(configChanged()));
    connect(differentRB,SIGNAL(toggled(bool)),this,SLOT(checkConfigureWm()));
    connect(wmCombo,SIGNAL(activated(int)),this,SLOT(checkConfigureWm()));
    connect(configureButton,SIGNAL(clicked()),this,SLOT(configureWm()));

    KGlobal::dirs()->addResourceType( "windowmanagers", "data", "ksmserver/windowmanagers" );
}

CfgWm::~CfgWm()
{
}

void CfgWm::configChanged()
{
    emit changed(true);
}

void CfgWm::defaults()
{
    wmCombo->setCurrentIndex( 0 );
}


void CfgWm::load(KConfig *)
{
    KConfig cfg("ksmserverrc", KConfig::NoGlobals);
    KConfigGroup c( &cfg, "General");
    loadWMs(c.readEntry("windowManager", KWIN_BIN));
    emit changed(false);
}

void CfgWm::save(KConfig *)
{
    saveAndConfirm();
}

bool CfgWm::saveAndConfirm()
{
    KConfig cfg("ksmserverrc", KConfig::NoGlobals);
    KConfigGroup c( &cfg, "General");
    c.writeEntry("windowManager", currentWm());
    emit changed(false);
    if( oldwm == currentWm())
        return true;
    QString restartArgument = currentWmData().restartArgument;
    if( restartArgument.isEmpty())
    {
        KMessageBox::information( this,
            i18n( "The new window manager will be used when KDE is started the next time." ),
            i18n( "Window Manager Change" ), "windowmanagerchange" );
        oldwm = currentWm();
        return true;
    }
    else
    {
        if( tryWmLaunch())
        {
            oldwm = currentWm();
            cfg.sync();
            QDBusInterface ksmserver("org.kde.ksmserver", "/KSMServer" );
            ksmserver.call( QDBus::NoBlock, "wmChanged" );
            KMessageBox::information( window(),
                i18n( "A new window manager is running.\n"
                    "It is still recommended to restart this KDE session to make sure "
                    "all running applications adjust for this change." ),
                    i18n( "Window Manager Replaced" ), "restartafterwmchange" );
            return true;
        }
        else
        { // revert config
            emit changed(true);
            c.writeEntry("windowManager", oldwm);
            if( oldwm == KWIN_BIN )
            {
                kwinRB->setChecked( true );
                wmCombo->setEnabled( false );
            }
            else
            {
                differentRB->setChecked( true );
                wmCombo->setEnabled( true );
                for( QHash< QString, WmData >::ConstIterator it = wms.constBegin();
                     it != wms.constEnd();
                     ++it )
                {
                    if( (*it).internalName == oldwm ) // make it selected
                        wmCombo->setCurrentIndex( wmCombo->findText( it.key()));
                }
            }
            return false;
        }
    }
}

bool CfgWm::tryWmLaunch()
{
    if( currentWm() == KWIN_BIN
        && qstrcmp( NETRootInfo( QX11Info::connection(), NET::SupportingWMCheck ).wmName(), "KWin" ) == 0 )
    {
        return true; // it is already running, don't necessarily restart e.g. after a failure with other WM
    }
    KMessageBox::information( window(), i18n( "Your running window manager will be now replaced with "
        "the configured one." ), i18n( "Window Manager Change" ), "windowmanagerchange" );
    wmLaunchingState = WmLaunching;
    wmProcess = new KProcess;
    *wmProcess << KShell::splitArgs( currentWmData().exec ) << currentWmData().restartArgument;
    connect( wmProcess, SIGNAL( error( QProcess::ProcessError )), this, SLOT( wmLaunchError()));
    connect( wmProcess, SIGNAL( finished( int, QProcess::ExitStatus )),
        this, SLOT( wmLaunchFinished( int, QProcess::ExitStatus )));
    wmProcess->start();
    wmDialog = new KTimerDialog( 20000, KTimerDialog::CountDown, window(), i18n( "Config Window Manager Change" ),
        KTimerDialog::Ok | KTimerDialog::Cancel, KTimerDialog::Cancel );
    wmDialog->setButtonGuiItem( KDialog::Ok, KGuiItem( i18n( "&Accept Change" ), "dialog-ok" ));
    wmDialog->setButtonGuiItem( KDialog::Cancel, KGuiItem( i18n( "&Revert to Previous" ), "dialog-cancel" ));
    QLabel *label = new QLabel(
        i18n( "The configured window manager is being launched.\n"
            "Please check it has started properly and confirm the change.\n"
            "The launch will be automatically reverted in 20 seconds." ), wmDialog );
    label->setWordWrap( true );
    wmDialog->setMainWidget( label );
    if( wmDialog->exec() == QDialog::Accepted ) // the user confirmed
        wmLaunchingState = WmOk;
    else // cancelled for some reason
        {
        if( wmLaunchingState == WmLaunching )
            { // time out
            wmLaunchingState = WmFailed;
            KProcess::startDetached( KWIN_BIN, QStringList() << "--replace" );
            // Let's hope KWin never fails.
            KMessageBox::sorry( window(),
                i18n( "The running window manager has been reverted to the default KDE window manager KWin." ));
            }
        else if( wmLaunchingState == WmFailed )
            {
            KProcess::startDetached( KWIN_BIN, QStringList() << "--replace" );
            // Let's hope KWin never fails.
            KMessageBox::sorry( window(),
                i18n( "The new window manager has failed to start.\n"
                    "The running window manager has been reverted to the default KDE window manager KWin." ));
            }
        }
    bool ret = ( wmLaunchingState == WmOk );
    wmLaunchingState = WmNone;
    delete wmDialog;
    wmDialog = NULL;
    // delete wmProcess; - it is intentionally leaked, since there is no KProcess:detach()
    wmProcess = NULL;
    return ret;
}

void CfgWm::wmLaunchError()
{
    if( wmLaunchingState != WmLaunching || sender() != wmProcess )
        return;
    wmLaunchingState = WmFailed;
    wmDialog->reject();
}


void CfgWm::wmLaunchFinished( int exitcode, QProcess::ExitStatus exitstatus )
{
    if( wmLaunchingState != WmLaunching || sender() != wmProcess )
        return;
    if( exitstatus == QProcess::NormalExit && exitcode == 0 )
        { // assume it's forked into background
        wmLaunchingState = WmOk;
        return;
        }
    // otherwise it's a failure
    wmLaunchingState = WmFailed;
    wmDialog->reject();
}

void CfgWm::loadWMs( const QString& current )
{
    WmData kwin;
    kwin.internalName = KWIN_BIN;
    kwin.exec = KWIN_BIN;
    kwin.configureCommand = "";
    kwin.restartArgument = "--replace";
    kwin.parentArgument = "";
    wms[ "KWin" ] = kwin;
    oldwm = KWIN_BIN;
    kwinRB->setChecked( true );
    wmCombo->setEnabled( false );

    QStringList list = KGlobal::dirs()->findAllResources( "windowmanagers", QString(), KStandardDirs::NoDuplicates );
    QRegExp reg( ".*/([^/\\.]*)\\.[^/\\.]*" );
    foreach( const QString& wmfile, list )
    {
        KDesktopFile file( wmfile );
        if( file.noDisplay())
            continue;
        if( !file.tryExec())
            continue;
        QString testexec = file.desktopGroup().readEntry( "X-KDE-WindowManagerTestExec" );
        if( !testexec.isEmpty())
        {
            KProcess proc;
            proc.setShellCommand( testexec );
            if( proc.execute() != 0 )
                continue;
        }
        QString name = file.readName();
        if( name.isEmpty())
            continue;
        if( !reg.exactMatch( wmfile ))
            continue;
        QString wm = reg.cap( 1 );
        if( wms.contains( name ))
            continue;
        WmData data;
        data.internalName = wm;
        data.exec = file.desktopGroup().readEntry( "Exec" );
        if( data.exec.isEmpty())
            continue;
        data.configureCommand = file.desktopGroup().readEntry( "X-KDE-WindowManagerConfigure" );
        data.restartArgument = file.desktopGroup().readEntry( "X-KDE-WindowManagerRestartArgument" );
        data.parentArgument = file.desktopGroup().readEntry( "X-KDE-WindowManagerConfigureParentArgument" );
        wms[ name ] = data;
        wmCombo->addItem( name );
        if( wms[ name ].internalName == current ) // make it selected
        {
            wmCombo->setCurrentIndex( wmCombo->count() - 1 );
            oldwm = wm;
            differentRB->setChecked( true );
            wmCombo->setEnabled( true );
        }
    }
    differentRB->setEnabled( wmCombo->count()>0 );
    checkConfigureWm();
}

CfgWm::WmData CfgWm::currentWmData() const
{
    return kwinRB->isChecked() ? wms[ "KWin" ] : wms[ wmCombo->currentText() ];
}

QString CfgWm::currentWm() const
{
    return currentWmData().internalName;
}

void CfgWm::checkConfigureWm()
{
    configureButton->setEnabled( !currentWmData().configureCommand.isEmpty());
}

void CfgWm::configureWm()
{
    if( oldwm != currentWm() // needs switching first
        && !saveAndConfirm())
    {
        return;
    }
    QStringList args;
    if( !currentWmData().parentArgument.isEmpty())
        args << currentWmData().parentArgument << QString::number( window()->winId());
    if( !KProcess::startDetached( currentWmData().configureCommand, args ))
        KMessageBox::sorry( window(), i18n( "Running the configuration tool failed" ));
}
