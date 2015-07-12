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

#include <kuser.h>
#include <kdialog.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "settings.h"
#include <KPluginFactory>
#include <KPluginLoader>

#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QIcon>
#include <QLineEdit>

static const int faceIconSize = 46;

K_PLUGIN_FACTORY(Factory,
        registerPlugin<KCMUserAccount>();
        )
K_EXPORT_PLUGIN(Factory("useraccount"))

KCMUserAccount::KCMUserAccount(QWidget *parent, const QVariantList &)
  : KCModule(parent),
    _accountList(NULL),
    _ku(NULL),
    _am(NULL),
    _currentUser(NULL),
    _currentFaceIcon(NULL),
    _currentFullName(NULL),
    _currentAccountType(NULL),
    _currentLanguage(NULL),
    _passwdEdit(NULL)
{
	QHBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setSpacing(KDialog::spacingHint());
    topLayout->setMargin(0);

    _accountList = new QListWidget;
    _accountList->setIconSize(QSize(faceIconSize, faceIconSize));
	connect(_accountList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    topLayout->addWidget(_accountList);

    QFormLayout *formLayout = new QFormLayout;
    topLayout->addLayout(formLayout);

    _currentFaceIcon = new QPushButton;
    _currentFaceIcon->setMinimumWidth(faceIconSize);
    _currentFaceIcon->setMinimumHeight(faceIconSize);
    _currentFaceIcon->setIconSize(QSize(faceIconSize, faceIconSize));
    _currentFullName = new QLabel("FullName");
    formLayout->addRow(_currentFaceIcon, _currentFullName);

    _currentAccountType = new QComboBox;
    _currentAccountType->addItem(i18n("Standard"));
    _currentAccountType->addItem(i18n("Administrator"));
    formLayout->addRow(i18n("Account Type"), _currentAccountType);

    _currentLanguage = new QLabel("English");
    formLayout->addRow(i18n("Language"), _currentLanguage);

    formLayout->addRow(new QLabel(i18n("Login Options")));

    _passwdEdit = new QLineEdit("password");
    _passwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    formLayout->addRow(i18n("Password"), _passwdEdit);

    QHBoxLayout *hbox = new QHBoxLayout;
    _autoLoginButton = new QRadioButton(i18n("Yes"));
    _nonAutoLoginButton = new QRadioButton(i18n("No"));
    hbox->addWidget(_autoLoginButton);
    hbox->addWidget(_nonAutoLoginButton);
    formLayout->addRow(i18n("Automatic Login"), hbox);

    KAboutData *about = new KAboutData("kcm_useraccount", i18n("Password & User Information"), "0.1",
        QString(), KAboutLicense::GPL, i18n("(C) 2015, Leslie Zhai"));

    about->addAuthor(i18n("Leslie Zhai"), i18n("Developer"), "xiang.zhai@i-soft.com.cn");
    setAboutData(about);

	setQuickHelp(i18n("<qt>Here you can change your personal information, which "
			"will be used, for instance, in mail programs and word processors. You can "
			"change your login password by clicking <em>Change Password...</em>.</qt>") );

    _ku = new KUser;

    _am = new AccountsService::AccountsManager;
}

KCMUserAccount::~KCMUserAccount()
{
    if (_accountList) {
        while (_accountList->count()) {
            _accountList->takeItem(0);
        }
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

    if (_am) {
        delete _am;
        _am = NULL;
    }
}

QIcon KCMUserAccount::_faceIcon(QString faceIconPath) 
{
    return QIcon(_facePixmap(faceIconPath));
}

QPixmap KCMUserAccount::_facePixmap(QString faceIconPath) 
{
    if (faceIconPath == "") {
        QPixmap pixmap(faceIconSize, faceIconSize);
        pixmap.fill();
        return pixmap;
    }

    return QPixmap(faceIconPath);
}

void KCMUserAccount::slotItemClicked(QListWidgetItem *item) 
{
    QString itemText = item->text();
    KUser user(itemText);
    
    _currentUser = _am->findUserByName(itemText);

    if (!user.isValid() || _currentUser == NULL) {
        return;
    }

    _currentFaceIcon->setIcon(_faceIcon(user.faceIconPath()));
    _currentFullName->setText(_currentUser->displayName());

    _currentAccountType->setCurrentText((int)_currentUser->accountType() ? 
                                        i18n("Administrator") : 
                                        i18n("Standard"));
    connect(_currentAccountType, SIGNAL(activated(int)), this, SLOT(changed()));

    _currentLanguage->setText(_currentUser->language());

    connect(_passwdEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changed()));

    _autoLoginButton->setChecked(_currentUser->automaticLogin());
    connect(_autoLoginButton, SIGNAL(clicked()), this, SLOT(changed()));
    _nonAutoLoginButton->setChecked(!_currentUser->automaticLogin());
    connect(_nonAutoLoginButton, SIGNAL(clicked()), this, SLOT(changed()));
}

void KCMUserAccount::load()
{
    while (_accountList->count()) {
        _accountList->takeItem(0);
    }
    
    QString myAccountLoginName = _ku->loginName();
    
    QListWidgetItem *item = new QListWidgetItem(i18n("My Account"));
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

    KCModule::load();
}

void KCMUserAccount::save() 
{
    KCModule::save();

    if (_currentUser == NULL) {
        return;
    }

    _currentUser->setAccountType((AccountsService::UserAccount::AccountType)_currentAccountType->currentIndex());

    // FIXME: QtAccountsService fail to setAutomaticLogin?
    _currentUser->setAutomaticLogin(_autoLoginButton->isChecked());
}

#include "main.moc"
