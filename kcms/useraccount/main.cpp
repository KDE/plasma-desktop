
/**
 *  Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *
 *
 *  Please see the README
 *
 */

#include "main.h"

#include <kpushbutton.h>
#include <kguiitem.h>
#include <kpassworddialog.h>
#include <kuser.h>
#include <kdialog.h>
#include <qicon.h>
#include <kimageio.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kurl.h>
#include <kdebug.h>

#include "settings.h"
#include "pass.h"
#include <KPluginFactory>
#include <KPluginLoader>

#include <QDBusInterface>
#include <QDBusReply>
#include <QDragEnterEvent>
#include <QEvent>
#include <QPixmap>
#include <QProcess>
#include <QStandardPaths>
#include <QStringList>


K_PLUGIN_FACTORY(Factory,
        registerPlugin<KCMUserAccount>();
        )
K_EXPORT_PLUGIN(Factory("useraccount"))

KCMUserAccount::KCMUserAccount( QWidget *parent, const QVariantList &)
    : KCModule(parent)
{
	QVBoxLayout *topLayout = new QVBoxLayout(this);
        topLayout->setSpacing(KDialog::spacingHint());
        topLayout->setMargin(0);

	_mw = new MainWidget(this);
	topLayout->addWidget( _mw );

	connect( _mw->btnChangeFace, SIGNAL(clicked()), SLOT(slotFaceButtonClicked()));
	connect( _mw->btnChangePassword, SIGNAL(clicked()), SLOT(slotChangePassword()));
	KGuiItem::assign(_mw->btnChangePassword, KGuiItem( i18n("Change &Password..."), "preferences-desktop-user-password" ));

	connect( _mw->leRealname, SIGNAL(textChanged(QString)), SLOT(changed()));
	connect( _mw->leOrganization, SIGNAL(textChanged(QString)), SLOT(changed()));
	connect( _mw->leEmail, SIGNAL(textChanged(QString)), SLOT(changed()));
	connect( _mw->leSMTP, SIGNAL(textChanged(QString)), SLOT(changed()));

	_ku = new KUser();
	_kes = new KEMailSettings();

	_mw->lblUsername->setText( _ku->loginName() );
	QFont font( _mw->lblUsername->font() );
	font.setPointSizeF( font.pointSizeF()  * 1.41 );
	font.setBold( true );
	_mw->lblUsername->setFont( font );
	_mw->lblUID->setText( QString().number(_ku->uid()) );

    KAboutData *about = new KAboutData("kcm_useraccount", i18n("Password & User Information"), "0.1",
        QString(), KAboutLicense::GPL, i18n("(C) 2002, Braden MacDonald, (C) 2004 Ravikiran Rajagopal"));

    about->addAuthor(i18n("Frans Englich"), i18n("Maintainer"), "frans.englich@telia.com");
    about->addAuthor(i18n("Ravikiran Rajagopal"), QString(), "ravi@kde.org");
    about->addAuthor(i18n("Michael H\303\244ckel"), QString(), "haeckel@kde.org" );

    about->addAuthor(i18n("Braden MacDonald"), i18n("Face editor"), "bradenm_k@shaw.ca");
    about->addAuthor(i18n("Geert Jansen"), i18n("Password changer"), "jansen@kde.org", "http://www.stack.nl/~geertj/");
    about->addAuthor(i18n("Daniel Molkentin"));
    about->addAuthor(i18n("Alex Zepeda"));
    about->addAuthor(i18n("Hans Karlsson"), i18n("Icons"), "karlsson.h@home.se");
    about->addAuthor(i18n("Hermann Thomas"), i18n("Icons"), "h.thomas@gmx.de");
    setAboutData(about);

	setQuickHelp( i18n("<qt>Here you can change your personal information, which "
			"will be used, for instance, in mail programs and word processors. You can "
			"change your login password by clicking <em>Change Password...</em>.</qt>") );

	addConfig( KCFGPassword::self(), this );
	load();
}

void KCMUserAccount::slotChangePassword()
{
	QString bin = KGlobal::dirs()->findExe("kdepasswd");
	if ( bin.isEmpty() )
	{
		kDebug() << "kcm_useraccount: kdepasswd was not found.";
		KMessageBox::sorry ( this, i18n( "A program error occurred: the internal "
			"program 'kdepasswd' could not be found. You will "
			"not be able to change your password."));

		_mw->btnChangePassword->setEnabled(false);
		return;
	}
	QStringList lst;
	lst << _ku->loginName();
	QProcess::startDetached(bin,lst);

}


KCMUserAccount::~KCMUserAccount()
{
	delete _ku;
	delete _kes;
}

void KCMUserAccount::load()
{
	_mw->lblUsername->setText(_ku->loginName());

	_kes->setProfile(_kes->defaultProfileName());

	QString realName = _kes->getSetting( KEMailSettings::RealName );

	if (realName.isEmpty()) {
		realName = _ku->property(KUser::FullName).toString();
	}

	_mw->leRealname->setText( realName );
	_mw->leEmail->setText( _kes->getSetting( KEMailSettings::EmailAddress ));
	_mw->leOrganization->setText( _kes->getSetting( KEMailSettings::Organization ));
	_mw->leSMTP->setText( _kes->getSetting( KEMailSettings::OutServer ));

	// load user face
	_facePixmap = QPixmap( KCFGUserAccount::faceFile() );
	_mw->btnChangeFace->setIcon( QIcon(_facePixmap) );
	if (!_facePixmap.isNull()) {
		_mw->btnChangeFace->setIconSize(_facePixmap.size().boundedTo(QSize(74,74)));
	}

	KCModule::load(); /* KConfigXT */

}

void KCMUserAccount::save()
{
	KCModule::save(); /* KConfigXT */

/*
 * FIXME: there is apparently no way to set full name
 * non-interactively as a normal user on FreeBSD.
 */
#ifndef Q_OS_FREEBSD
	/* Save realname to /etc/passwd */
	if ( _mw->leRealname->isModified() )
	{
        // save icon file also with accountsservice
        QDBusInterface ainterface("org.freedesktop.Accounts",
            "/org/freedesktop/Accounts",
            "org.freedesktop.Accounts",
            QDBusConnection::systemBus());
        QDBusReply<QDBusObjectPath> reply = ainterface.call("FindUserById", qlonglong(_ku->uid()));
        if (reply.isValid() && !reply.error().isValid()) {
            QDBusInterface uinterface("org.freedesktop.Accounts",
                reply.value().path(),
                "org.freedesktop.Accounts.User",
                QDBusConnection::systemBus(),
                this);

            QString name = _mw->leRealname->text();
            QDBusReply<void> ureply = uinterface.call("SetRealName", name);
            if (!ureply.isValid() || ureply.error().isValid()) {
                kDebug() << ureply.error().message();
                KMessageBox::error( this, i18n("There was an error setting the name: %1" ,
                    name) );
            }
        }
	}
#endif

	/* Save the image */
	if( !_facePixmap.isNull() )
	{
		if( !_facePixmap.save( KCFGUserAccount::faceFile(), "PNG" ))
		{
			KMessageBox::error( this, i18n("There was an error saving the image: %1" ,
				KCFGUserAccount::faceFile()) );
		}
		// save icon file also with accountsservice
		QDBusInterface ainterface("org.freedesktop.Accounts",
			"/org/freedesktop/Accounts",
			"org.freedesktop.Accounts",
			QDBusConnection::systemBus());
		QDBusReply<QDBusObjectPath> reply = ainterface.call("FindUserById", qlonglong(_ku->uid()));
		if (reply.isValid()) {
			QDBusInterface uinterface("org.freedesktop.Accounts",
				reply.value().path(),
				"org.freedesktop.Accounts.User",
				QDBusConnection::systemBus(),
				this);
			QDBusReply<void> ureply = uinterface.call("SetIconFile", KCFGUserAccount::faceFile());
			if (!ureply.isValid()) {
				kDebug() << ureply.error().message();
				KMessageBox::error( this, i18n("There was an error setting the image: %1" ,
					KCFGUserAccount::faceFile()) );
			}
		}
	}
	else { // delete existing image
        if (QFile::exists(KCFGUserAccount::faceFile())) {
            if ( !KIO::NetAccess::del(KCFGUserAccount::faceFile(), this) ) {
                KMessageBox::error( this, i18n("There was an error deleting the image: %1" ,
                    KCFGUserAccount::faceFile()) );
            }
        }
	}

	/* Save KDE's homebrewn settings */
	_kes->setSetting( KEMailSettings::RealName, _mw->leRealname->text() );
	_kes->setSetting( KEMailSettings::EmailAddress, _mw->leEmail->text() );
	_kes->setSetting( KEMailSettings::Organization, _mw->leOrganization->text() );
	_kes->setSetting( KEMailSettings::OutServer, _mw->leSMTP->text() );
}

void KCMUserAccount::changeFace(const QPixmap &pix)
{
  _facePixmap = pix;
  _mw->btnChangeFace->setIcon( QIcon(_facePixmap) );
  if ( !_facePixmap.isNull() )
    _mw->btnChangeFace->setIconSize(_facePixmap.size());
  emit changed( true );
}

void KCMUserAccount::slotFaceButtonClicked()
{
  QString picsdir = QStandardPaths::locate( QStandardPaths::GenericDataLocation,
                                            "kdm/pics/users/",
                                            QStandardPaths::LocateDirectory );
  ChFaceDlg* pDlg = new ChFaceDlg( picsdir, this );

  if ( pDlg->exec() == QDialog::Accepted )
      changeFace( pDlg->getFaceImage() );

  delete pDlg;
}

/**
 * I merged faceButtonDropEvent into this /Frans
 * The function was called after checking event type and
 * the code is now below that if statement
 */
bool KCMUserAccount::eventFilter(QObject *, QEvent *e)
{
	if (e->type() == QEvent::DragEnter)
		{
		QDragEnterEvent *ee = (QDragEnterEvent *) e;
    if (!KUrl::List::fromMimeData( ee->mimeData() ).isEmpty())
      ee->accept();
    else
      ee->ignore();
		return true;
	}

	if (e->type() == QEvent::Drop)
	{
		KUrl *url = decodeImgDrop( (QDropEvent *) e, this);
		if (url)
		{
			QString pixPath;
			KIO::NetAccess::download(*url, pixPath, this);
			changeFace( QPixmap( pixPath ) );
			KIO::NetAccess::removeTempFile(pixPath);
			delete url;
		}
		return true;
	}
	return false;
}

inline KUrl *KCMUserAccount::decodeImgDrop(QDropEvent *e, QWidget *wdg)
{
  KUrl::List uris = KUrl::List::fromMimeData(e->mimeData());
  if (!uris.isEmpty())
  {
    KUrl *url = new KUrl(uris.first());

    KMimeType::Ptr mime = KMimeType::findByUrl( *url );
    if ( mime && KImageIO::isSupported( mime->name(), KImageIO::Reading ) )
      return url;

    QStringList qs = KImageIO::pattern().split( '\n');
    qs.erase(qs.begin());

    QString msg = i18n( "%1 does not appear to be an image file.\n"
			  "Please use files with these extensions:\n"
			  "%2", url->fileName(), qs.join("\n"));
    KMessageBox::sorry( wdg, msg);
    delete url;
  }
  return 0;
}

#include "main.moc"

