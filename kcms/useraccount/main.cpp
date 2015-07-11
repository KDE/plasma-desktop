/**
 *  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
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
 */

#include "main.h"

//#include <kpushbutton.h>
//#include <kguiitem.h>
//#include <kpassworddialog.h>
#include <kuser.h>
#include <kdialog.h>
//#include <qicon.h>
//#include <kimageio.h>
//#include <kmimetype.h>
//#include <kstandarddirs.h>
#include <kaboutdata.h>
//#include <kmessagebox.h>
//#include <kio/netaccess.h>
//#include <kurl.h>
#include <kdebug.h>

#include "settings.h"
#include <KPluginFactory>
#include <KPluginLoader>

#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QIcon>
#include <QLineEdit>

K_PLUGIN_FACTORY(Factory,
        registerPlugin<KCMUserAccount>();
        )
K_EXPORT_PLUGIN(Factory("useraccount"))

KCMUserAccount::KCMUserAccount(QWidget* parent, const QVariantList&)
  : KCModule(parent),
    _accountList(NULL),
    _ku(NULL),
    _currentFaceIcon(NULL),
    _currentFullName(NULL),
    _currentAccountType(NULL)
{
	QHBoxLayout* topLayout = new QHBoxLayout(this);
    topLayout->setSpacing(KDialog::spacingHint());
    topLayout->setMargin(0);

    _accountList = new QListWidget;
    _accountList->setIconSize(QSize(46, 46));
	connect(_accountList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    topLayout->addWidget(_accountList);

    QFormLayout* formLayout = new QFormLayout;
    topLayout->addLayout(formLayout);

    _currentFaceIcon = new QLabel;
    _currentFullName = new QLabel("FullName");
    formLayout->addRow(_currentFaceIcon, _currentFullName);

    _currentAccountType = new QLabel("Administrator");
    formLayout->addRow(i18n("Account Type"), _currentAccountType);

    formLayout->addRow(new QLabel(i18n("Login Options")));

    QLineEdit* passwdEdit = new QLineEdit("password");
    passwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    formLayout->addRow(i18n("Password"), passwdEdit);

    KAboutData *about = new KAboutData("kcm_useraccount", i18n("Password & User Information"), "0.1",
        QString(), KAboutLicense::GPL, i18n("(C) 2015, Leslie Zhai"));

    about->addAuthor(i18n("Leslie Zhai"), i18n("Developer"), "xiang.zhai@i-soft.com.cn");
    setAboutData(about);

	setQuickHelp(i18n("<qt>Here you can change your personal information, which "
			"will be used, for instance, in mail programs and word processors. You can "
			"change your login password by clicking <em>Change Password...</em>.</qt>") );

    _ku = new KUser;
}

KCMUserAccount::~KCMUserAccount()
{
    if (_accountList) {
        delete _accountList;
        _accountList = NULL;
    }

    if (_currentFaceIcon) {
        delete _currentFaceIcon;
        _currentFaceIcon = NULL;
    }

    if (_currentFullName) {
        delete _currentFullName;
        _currentFullName = NULL;
    }

    if (_currentAccountType) {
        delete _currentAccountType;
        _currentAccountType = NULL;
    }

    if (_ku) {
        delete _ku;
        _ku = NULL;
    }
}

QIcon KCMUserAccount::_faceIcon(QString faceIconPath) 
{
    return QIcon(_facePixmap(faceIconPath));
}

QPixmap KCMUserAccount::_facePixmap(QString faceIconPath) 
{
    if (faceIconPath == "") {
        QPixmap pixmap(46, 46);
        pixmap.fill();
        return pixmap;
    }

    return QPixmap(faceIconPath);
}

void KCMUserAccount::slotItemClicked(QListWidgetItem* item) 
{
    QString itemText = item->text();
    KUser user(itemText);

    if (!user.isValid())
        return;

    _currentFaceIcon->setPixmap(_facePixmap(user.faceIconPath()));
    _currentFullName->setText(user.fullName() != "" ? user.fullName() : user.loginName());

    _currentAccountType->setText(user.isSuperUser() ? "Administrator" : "Standard");
}

void KCMUserAccount::load()
{
    QString myAccountLoginName = _ku->loginName();
    
    QListWidgetItem* item = new QListWidgetItem(i18n("My Account"));
    _accountList->addItem(item);

    item = new QListWidgetItem(QIcon(QPixmap(KCFGUserAccount::faceFile())), 
                               myAccountLoginName);
    _accountList->addItem(item);
    _accountList->setCurrentItem(item);
    slotItemClicked(item);

    item = new QListWidgetItem(i18n("Other Accounts"));
    _accountList->addItem(item);

    foreach (KUser user, _ku->allUsers()) {
        if (user.loginName() != myAccountLoginName && 
            user.homeDir().startsWith("/home")) {
            item = new QListWidgetItem(_faceIcon(user.faceIconPath()), 
                                       user.loginName());
            _accountList->addItem(item);
        }
    }
}

#include "main.moc"
