/*
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "knetattach.h"

#include <QUrlQuery>
#include <QVariant>

#include <KCharsets>
#include <KConfig>
#include <KConfigGroup>
#include <KDirNotify>
#include <KIO/StatJob>
#include <KJobWidgets>
#include <KMessageBox>
#include <KRun>
#include <QDesktopServices>
#include <QIcon>
#include <QTextCodec>

KNetAttach::KNetAttach(QWidget *parent)
    : QWizard(parent)
    , Ui_KNetAttach()
{
    setupUi(this);

    connect(_recent, &QAbstractButton::toggled, _recentConnectionName, &QWidget::setEnabled);
    connect(_connectionName, &QLineEdit::textChanged, this, &KNetAttach::updateParametersPageStatus);
    connect(_user, &QLineEdit::textChanged, this, &KNetAttach::updateParametersPageStatus);
    connect(_host, &QLineEdit::textChanged, this, &KNetAttach::updateParametersPageStatus);
    connect(_port, (void (QSpinBox::*)(int)) & QSpinBox::valueChanged, this, &KNetAttach::updateParametersPageStatus);
    connect(_path, &QLineEdit::textChanged, this, &KNetAttach::updateParametersPageStatus);
    connect(_useEncryption, &QAbstractButton::toggled, this, &KNetAttach::updatePort);
    connect(_createIcon, &QAbstractButton::toggled, this, &KNetAttach::updateFinishButtonText);
    connect(this, &QWizard::helpRequested, this, &KNetAttach::slotHelpClicked);
    connect(this, &QWizard::currentIdChanged, this, &KNetAttach::slotPageChanged);
    setWindowIcon(QIcon::fromTheme(QStringLiteral("knetattach")));
    setOption(HaveHelpButton, true);
    // setResizeMode(Fixed); FIXME: make the wizard fixed-geometry
    button(FinishButton)->setEnabled(false);
    KConfig crecent(QStringLiteral("krecentconnections"), KConfig::NoGlobals);
    KConfigGroup recent(&crecent, "General");
    QStringList idx = recent.readEntry("Index", QStringList());
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
    const int codecForLocaleIdx = _encoding->findText(QString::fromLatin1(QTextCodec::codecForLocale()->name()), Qt::MatchContains);
    _encoding->setCurrentIndex(codecForLocaleIdx != -1 ? codecForLocaleIdx : 0);
}

void KNetAttach::slotPageChanged(int)
{
    updateFinishButtonText(true);
}

void KNetAttach::slotHelpClicked()
{
    QDesktopServices::openUrl(QUrl(QStringLiteral("help:/")));
}

void KNetAttach::setInformationText(const QString &type)
{
    QString text;

    if (type == QLatin1String("WebFolder")) {
        text =
            i18n("Enter a name for this <i>WebFolder</i> as well as a server address, port and folder path to use and press the <b>Save & Connect</b> button.");
    } else if (type == QLatin1String("Fish")) {
        text = i18n(
            "Enter a name for this <i>Secure shell connection</i> as well as a server address, port and folder path to use and press the <b>Save & Connect</b> "
            "button.");
    } else if (type == QLatin1String("FTP")) {
        text = i18n(
            "Enter a name for this <i>File Transfer Protocol connection</i> as well as a server address and folder path to use and press the <b>Save & "
            "Connect</b> button.");
    } else if (type == QLatin1String("SMB")) {
        text = i18n(
            "Enter a name for this <i>Microsoft Windows network drive</i> as well as a server address and folder path to use and press the <b>Save & "
            "Connect</b> button.");
    }

    _informationText->setText(text);
}

void KNetAttach::updateParametersPageStatus()
{
    button(FinishButton)->setEnabled(!_host->text().trimmed().isEmpty() && !_path->text().trimmed().isEmpty() && !_connectionName->text().trimmed().isEmpty());
}

bool KNetAttach::validateCurrentPage()
{
    if (currentPage() == _folderType) {
        _host->setFocus();
        _connectionName->setFocus();

        if (_webfolder->isChecked()) {
            setInformationText(QStringLiteral("WebFolder"));
            updateForProtocol(QStringLiteral("WebFolder"));
            _port->setValue(80);
        } else if (_fish->isChecked()) {
            setInformationText(QStringLiteral("Fish"));
            updateForProtocol(QStringLiteral("Fish"));
            _port->setValue(22);
        } else if (_ftp->isChecked()) {
            setInformationText(QStringLiteral("FTP"));
            updateForProtocol(QStringLiteral("FTP"));
            _port->setValue(21);
            if (_path->text().isEmpty()) {
                _path->setText(QStringLiteral("/"));
            }
        } else if (_smb->isChecked()) {
            setInformationText(QStringLiteral("SMB"));
            updateForProtocol(QStringLiteral("SMB"));
        } else { // if (_recent->isChecked()) {
            KConfig recent(QStringLiteral("krecentconnections"), KConfig::NoGlobals);
            if (!recent.hasGroup(_recentConnectionName->currentText())) {
                KConfigGroup group = recent.group("General");
                QStringList idx = group.readEntry("Index", QStringList());
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
            QUrl u(group.readEntry("URL"));
            _host->setText(u.host());
            _user->setText(u.userName());
            _path->setText(u.path());
            if (group.hasKey("Port")) {
                _port->setValue(group.readEntry("Port", 0));
            } else {
                _port->setValue(u.port());
            }
            _connectionName->setText(_recentConnectionName->currentText());
            _createIcon->setChecked(false);
        }
        updateParametersPageStatus();

    } else {
        button(BackButton)->setEnabled(false);
        button(FinishButton)->setEnabled(false);
        QUrl url;
        if (_type == QLatin1String("WebFolder")) {
            if (_useEncryption->isChecked()) {
                url.setScheme(QStringLiteral("webdavs"));
            } else {
                url.setScheme(QStringLiteral("webdav"));
            }
            url.setPort(_port->value());
        } else if (_type == QLatin1String("Fish")) {
            KConfig config(QStringLiteral("kio_fishrc"));
            KConfigGroup cg(&config, _host->text().trimmed());
            cg.writeEntry("Charset", KCharsets::charsets()->encodingForName(_encoding->currentText()));
            url.setScheme(_protocolText->currentText());
            url.setPort(_port->value());
        } else if (_type == QLatin1String("FTP")) {
            url.setScheme(QStringLiteral("ftp"));
            url.setPort(_port->value());
            KConfig config(QStringLiteral("kio_ftprc"));
            KConfigGroup cg(&config, _host->text().trimmed());
            cg.writeEntry("Charset", KCharsets::charsets()->encodingForName(_encoding->currentText()));
            config.sync();
        } else if (_type == QLatin1String("SMB")) {
            url.setScheme(QStringLiteral("smb"));
        } else { // recent
        }

        url.setHost(_host->text().trimmed());
        const QString trimmedUser = _user->text().trimmed();
        if (!trimmedUser.isEmpty()) {
            url.setUserName(trimmedUser);
        }
        QString path = _path->text().trimmed();
#ifndef Q_WS_WIN
        // could a relative path really be made absolute by simply prepending a '/' ?
        if (!path.startsWith(QLatin1Char('/'))) {
            path = QLatin1Char('/') + path;
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

        KRun::RunFlags flags;
        flags |= KRun::RunExecutables;
        KRun::runUrl(url, QStringLiteral("inode/directory"), this, flags);

        QString name = _connectionName->text().trimmed();

        if (_createIcon->isChecked()) {
            QString path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/remoteview/");
            path += name + QStringLiteral(".desktop");
            KConfig _desktopFile(path, KConfig::SimpleConfig);
            KConfigGroup desktopFile(&_desktopFile, "Desktop Entry");
            desktopFile.writeEntry("Icon", "folder-remote");
            desktopFile.writeEntry("Name", name);
            desktopFile.writeEntry("Type", "Link");
            desktopFile.writeEntry("URL", url.toDisplayString());
            desktopFile.writeEntry("Charset", QUrlQuery(url).queryItemValue(QStringLiteral("charset")));
            desktopFile.sync();
            org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("remote:/")));
        }

        if (!name.isEmpty()) {
            KConfig _recent(QStringLiteral("krecentconnections"), KConfig::NoGlobals);
            KConfigGroup recent(&_recent, "General");
            QStringList idx = recent.readEntry("Index", QStringList());
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
            recent = KConfigGroup(&_recent, name);
            recent.writeEntry("URL", url.toDisplayString());
            if (_type == QLatin1String("WebFolder") || _type == QLatin1String("Fish") || _type == QLatin1String("FTP")) {
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

bool KNetAttach::doConnectionTest(const QUrl &url)
{
    KIO::StatJob *job = KIO::stat(url);
    KJobWidgets::setWindow(job, this);
    return job->exec();
}

bool KNetAttach::updateForProtocol(const QString &protocol)
{
    _type = protocol;
    if (protocol == QLatin1String("WebFolder")) {
        _useEncryption->show();
        _portText->show();
        _port->show();
        _protocol->hide();
        _protocolText->hide();
        _userText->show();
        _user->show();
        _encodingText->hide();
        _encoding->hide();
    } else if (protocol == QLatin1String("Fish")) {
        _useEncryption->hide();
        _portText->show();
        _port->show();
        _protocol->show();
        _protocolText->show();
        _userText->show();
        _user->show();
        _encodingText->show();
        _encoding->show();
    } else if (protocol == QLatin1String("FTP")) {
        _useEncryption->hide();
        _portText->show();
        _port->show();
        _protocol->hide();
        _protocolText->hide();
        _userText->show();
        _user->show();
        _encodingText->show();
        _encoding->show();
    } else if (protocol == QLatin1String("SMB")) {
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
        _type = QString();
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
