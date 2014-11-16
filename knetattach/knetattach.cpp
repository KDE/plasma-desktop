/*
   Copyright (C) 2004 George Staikos <staikos@kde.org>

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


#include "knetattach.h"

#include <QtCore/QVariant>

#include <KIO/NetAccess>
#include <KMessageBox>
#include <QIcon>
#include <KLocale>
#include <KGlobalSettings>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KDirNotify>
#include <KCharsets>
#include <KDebug>
#include <KUrl>
#include <KRun>
#include <KToolInvocation>
#include <KGlobal>
#include <QDesktopServices>
#include <QTextCodec>

KNetAttach::KNetAttach( QWidget* parent )
    : QWizard( parent ), Ui_KNetAttach()
{
    setupUi( this );

    connect(_recent, SIGNAL(toggled(bool)), _recentConnectionName, SLOT(setEnabled(bool)));
    connect(_connectionName, SIGNAL(textChanged(QString)), this, SLOT(updateParametersPageStatus()));
    connect(_user, SIGNAL(textChanged(QString)), this, SLOT(updateParametersPageStatus()));
    connect(_host, SIGNAL(textChanged(QString)), this, SLOT(updateParametersPageStatus()));
    connect(_path, SIGNAL(textChanged(QString)), this, SLOT(updateParametersPageStatus()));
    connect(_useEncryption, SIGNAL(toggled(bool)), this, SLOT(updatePort(bool)));
    connect(_createIcon, SIGNAL(toggled(bool)), this, SLOT(updateFinishButtonText(bool)));
    connect( this, SIGNAL(helpRequested()), this, SLOT(slotHelpClicked()) );
    connect( this, SIGNAL(currentIdChanged(int)), this, SLOT(slotPageChanged(int)) );
    setWindowIcon(QIcon::fromTheme("knetattach"));
    setOption(HaveHelpButton, true);
    //setResizeMode(Fixed); FIXME: make the wizard fixed-geometry
    button(FinishButton)->setEnabled(false);
    KConfig crecent( "krecentconnections", KConfig::NoGlobals  );
    KConfigGroup recent(&crecent, "General");
    QStringList idx = recent.readEntry("Index",QStringList());
    if (idx.isEmpty()) {
	_recent->setEnabled(false);
	if (_recent->isChecked()) {
	    _webfolder->setChecked(true);
	}
    } else {
	_recent->setEnabled(true);
	_recentConnectionName->addItems(idx);
    }
    _encoding->clear();
    _encoding->addItems(KCharsets::charsets()->descriptiveEncodingNames());
    const int codecForLocaleIdx = _encoding->findText(QTextCodec::codecForLocale()->name(), Qt::MatchContains);
    _encoding->setCurrentIndex(codecForLocaleIdx != -1 ? codecForLocaleIdx : 0);
}

void KNetAttach::slotPageChanged( int )
{
    updateFinishButtonText(true);
}

void KNetAttach::slotHelpClicked()
{
    QDesktopServices::openUrl(QUrl("help:/knetattach"));
}

void KNetAttach::setInformationText( const QString &type )
{
    QString text;

    if (type==QLatin1String("WebFolder")) {
	text = i18n("Enter a name for this <i>WebFolder</i> as well as a server address, port and folder path to use and press the <b>Save & Connect</b> button.");
    } else if (type==QLatin1String("Fish")) {
	text = i18n("Enter a name for this <i>Secure shell connection</i> as well as a server address, port and folder path to use and press the <b>Save & Connect</b> button.");
    } else if (type==QLatin1String("FTP")) {
        text = i18n("Enter a name for this <i>File Transfer Protocol connection</i> as well as a server address and folder path to use and press the <b>Save & Connect</b> button.");
    } else if (type==QLatin1String("SMB")) {
        text = i18n("Enter a name for this <i>Microsoft Windows network drive</i> as well as a server address and folder path to use and press the <b>Save & Connect</b> button.");
    }

    _informationText->setText(text);
}

void KNetAttach::updateParametersPageStatus()
{
    button(FinishButton)->setEnabled(
		  !_host->text().trimmed().isEmpty() &&
		  !_path->text().trimmed().isEmpty() &&
		  !_connectionName->text().trimmed().isEmpty());
}

bool KNetAttach::validateCurrentPage()
{
    if (currentPage() == _folderType){
	_host->setFocus();
	_connectionName->setFocus();

	if (_webfolder->isChecked()) {
	    setInformationText("WebFolder");
	    updateForProtocol("WebFolder");
	    _port->setValue(80);
	} else if (_fish->isChecked()) {
	    setInformationText("Fish");
	    updateForProtocol("Fish");
	    _port->setValue(22);
	} else if (_ftp->isChecked()) {
	    setInformationText("FTP");
	    updateForProtocol("FTP");
	    _port->setValue(21);
	    if (_path->text().isEmpty()) {
		_path->setText("/");
	    }
	} else if (_smb->isChecked()) {
	    setInformationText("SMB");
	    updateForProtocol("SMB");
	} else { //if (_recent->isChecked()) {
	    KConfig recent( "krecentconnections", KConfig::NoGlobals );
	    if (!recent.hasGroup(_recentConnectionName->currentText())) {
		KConfigGroup group = recent.group("General");
		QStringList idx = group.readEntry("Index",QStringList());
		if (idx.isEmpty()) {
		    _recent->setEnabled(false);
		    if (_recent->isChecked()) {
			_webfolder->setChecked(true);
		    }
		} else {
		    _recent->setEnabled(true);
		    _recentConnectionName->addItems(idx);
		}
		return false;
	    }
	    KConfigGroup group = recent.group(_recentConnectionName->currentText());
	    _type = group.readEntry("Type");
	    setInformationText(_type);
	    if (!updateForProtocol(_type)) {
		// FIXME: handle error
	    }
	    KUrl u(group.readEntry("URL"));
	    _host->setText(u.host());
	    _user->setText(u.user());
	    _path->setText(u.path());
	    if (group.hasKey("Port")) {
		_port->setValue(group.readEntry("Port",0));
	    } else {
		_port->setValue(u.port());
	    }
	    _connectionName->setText(_recentConnectionName->currentText());
	    _createIcon->setChecked(false);
	}
	updateParametersPageStatus();

    }else{
      button(BackButton)->setEnabled(false);
      button(FinishButton)->setEnabled(false);
      KUrl url;
      if (_type == "WebFolder") {
	  if (_useEncryption->isChecked()) {
	      url.setProtocol("webdavs");
	  } else {
	      url.setProtocol("webdav");
	  }
	  url.setPort(_port->value());
      } else if (_type == "Fish") {
      KConfig config("kio_fishrc");
      KConfigGroup cg(&config, _host->text().trimmed());
      cg.writeEntry("Charset", KCharsets::charsets()->encodingForName(_encoding->currentText()));
	  url.setProtocol(_protocolText->currentText());
	  url.setPort(_port->value());
      } else if (_type == "FTP") {
	  url.setProtocol("ftp");
	  url.setPort(_port->value());
      KConfig config("kio_ftprc");
      KConfigGroup cg(&config, _host->text().trimmed());
      cg.writeEntry("Charset", KCharsets::charsets()->encodingForName(_encoding->currentText()));
      config.sync();
      } else if (_type == "SMB") {
	  url.setProtocol("smb");
      } else { // recent
      }

      url.setHost(_host->text().trimmed());
      url.setUser(_user->text().trimmed());
      QString path = _path->text().trimmed();
  #ifndef Q_WS_WIN
      // could a relative path really be made absolute by simply prepending a '/' ?
      if (!path.startsWith('/')) {
	  path = QString("/") + path;
      }
  #endif
      url.setPath(path);
    _folderParameters->setEnabled(false);
      bool success = doConnectionTest(url);
    _folderParameters->setEnabled(true);
      if (!success) {
	  KMessageBox::sorry(this, i18n("Unable to connect to server.  Please check your settings and try again."));
	  button(BackButton)->setEnabled(true);
	  return false;
      }

      KRun::runUrl(url, "inode/directory", this);

      QString name = _connectionName->text().trimmed();

      if (_createIcon->isChecked()) {
	  KGlobal::dirs()->addResourceType("remote_entries", "data", "remoteview");

	  QString path = KGlobal::dirs()->saveLocation("remote_entries");
	  path += name + ".desktop";
	  KConfig _desktopFile( path, KConfig::SimpleConfig );
	  KConfigGroup desktopFile(&_desktopFile, "Desktop Entry");
	  desktopFile.writeEntry("Icon", "folder-remote");
	  desktopFile.writeEntry("Name", name);
	  desktopFile.writeEntry("Type", "Link");
	  desktopFile.writeEntry("URL", url.prettyUrl());
      desktopFile.writeEntry("Charset", url.fileEncoding());
	  desktopFile.sync();
	  org::kde::KDirNotify::emitFilesAdded( QUrl("remote:/") );
      }

      if (!name.isEmpty()) {
	  KConfig _recent("krecentconnections", KConfig::NoGlobals);
	  KConfigGroup recent(&_recent, "General");
	  QStringList idx = recent.readEntry("Index",QStringList());
	  _recent.deleteGroup(name); // erase anything stale
	  if (idx.contains(name)) {
	      idx.removeAll(name);
	      idx.prepend(name);
	      recent.writeEntry("Index", idx);
	  } else {
	      QString last;
	      if (!idx.isEmpty()) {
		  last = idx.last();
		  idx.pop_back();
	      }
	      idx.prepend(name);
	      _recent.deleteGroup(last);
	      recent.writeEntry("Index", idx);
	  }
	recent = KConfigGroup(&_recent,name);
	  recent.writeEntry("URL", url.prettyUrl());
	  if (_type == "WebFolder" || _type == "Fish" || _type == "FTP") {
	      recent.writeEntry("Port", _port->value());
	  }
	  recent.writeEntry("Type", _type);
	  recent.sync();
      }
    }
    return true;
}


void KNetAttach::updatePort(bool encryption)
{
    if (_webfolder->isChecked()) {
	if (encryption) {
	    _port->setValue(443);
	} else {
	    _port->setValue(80);
	}
    }
}


bool KNetAttach::doConnectionTest(const KUrl& url)
{
    KIO::UDSEntry entry;
    if (KIO::NetAccess::stat(url, entry, this)) {
	// Anything to test here?
	return true;
    }
    return false;
}


bool KNetAttach::updateForProtocol(const QString& protocol)
{
    _type = protocol;
    if (protocol == "WebFolder") {
	_useEncryption->show();
	_portText->show();
	_port->show();
	_protocol->hide();
	_protocolText->hide();
	_userText->show();
	_user->show();
    _encodingText->hide();
    _encoding->hide();
    } else if (protocol == "Fish") {
	_useEncryption->hide();
	_portText->show();
	_port->show();
	_protocol->show();
	_protocolText->show();
	_userText->show();
	_user->show();
	_encodingText->show();
	_encoding->show();
    } else if (protocol == "FTP") {
	_useEncryption->hide();
	_portText->show();
	_port->show();
	_protocol->hide();
	_protocolText->hide();
	_userText->show();
	_user->show();
    _encodingText->show();
    _encoding->show();
    } else if (protocol == "SMB") {
	_useEncryption->hide();
	_portText->hide();
	_port->hide();
	_protocol->hide();
	_protocolText->hide();
	_userText->hide();
	_user->hide();
    _encodingText->hide();
    _encoding->hide();
    } else {
	_type = "";
	return false;
    }
    return true;
}


void KNetAttach::updateFinishButtonText(bool save)
{
    if (save) {
	button(FinishButton)->setText(i18n("Save && C&onnect"));
    } else {
	button(FinishButton)->setText(i18n("C&onnect"));
    }
}

// vim: ts=8 sw=4 noet
