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
#include "faceiconpopup.h"
#include "chgpwddlg.h"
#include "adduserdlg.h"
#include "removeuserdlg.h"

#include <kuser.h>
#include <kdialog.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include "settings.h"
#include <KPluginFactory>
#include <KPluginLoader>

#include <PolkitQt1/Gui/ActionButton>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QIcon>
#include <QLineEdit>
#include <QPushButton>

static const int opIconSize = 32;

K_PLUGIN_FACTORY(Factory,
        registerPlugin<KCMUserAccount>();
        )
K_EXPORT_PLUGIN(Factory("useraccount"))

KCMUserAccount::KCMUserAccount(QWidget *parent, const QVariantList &)
  : KCModule(parent),
    _accountList(NULL),
    _removeBtn(NULL),
    _ku(NULL),
    _am(NULL),
    _currentUser(NULL),
    _currentFaceIcon(NULL),
    _currentFullName(NULL),
    _currentAccountType(NULL),
    _currentLanguage(NULL),
    _passwdEdit(NULL),
    _editUserName("")
{
	QHBoxLayout *topLayout = new QHBoxLayout(this);
    topLayout->setSpacing(KDialog::spacingHint());
    topLayout->setMargin(0);

    QVBoxLayout *vbox = new QVBoxLayout;
    _accountList = new QListWidget;
    _accountList->setIconSize(QSize(faceIconSize, faceIconSize));
    _accountList->setMaximumWidth(206);
    connect(_accountList, SIGNAL(itemClicked(QListWidgetItem*)), 
            this, SLOT(slotItemClicked(QListWidgetItem*)));
    vbox->addWidget(_accountList);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->setAlignment(Qt::AlignLeft);
    QPushButton *addBtn = new QPushButton;
    addBtn->setMaximumWidth(opIconSize);
    addBtn->setMaximumHeight(opIconSize);
    addBtn->setIcon(QIcon::fromTheme("list-add"));
    connect(addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
    hbox->addWidget(addBtn);
    _removeBtn = new QPushButton;
    _removeBtn->setEnabled(false);
    _removeBtn->setMaximumWidth(opIconSize);
    _removeBtn->setMaximumHeight(opIconSize);
    _removeBtn->setIcon(QIcon::fromTheme("list-remove"));
    connect(_removeBtn, SIGNAL(clicked()), this, SLOT(slotRemoveBtnClicked()));
    hbox->addWidget(_removeBtn);
    vbox->addLayout(hbox);
    topLayout->addLayout(vbox);

    topLayout->addSpacing(22);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->setLabelAlignment(Qt::AlignLeft);
    formLayout->setHorizontalSpacing(30);
    formLayout->setVerticalSpacing(18);
    topLayout->addLayout(formLayout);

    QPushButton *unlockBtn = new QPushButton;
    unlockBtn->setMaximumWidth(opIconSize);
    unlockBtn->setMaximumHeight(opIconSize);
    unlockBtn->setIcon(QIcon::fromTheme("action-unlocked"));
    PolkitQt1::Gui::ActionButton *actionBtn = new PolkitQt1::Gui::ActionButton(
        unlockBtn, "org.freedesktop.accounts.user-administration", this);
    formLayout->addRow(unlockBtn);

    _currentFaceIcon = new FaceIconButton;
    _currentFaceIcon->setMinimumWidth(faceIconSize);
    _currentFaceIcon->setMinimumHeight(faceIconSize);
    _currentFaceIcon->setIconSize(QSize(faceIconSize, faceIconSize));
    connect(_currentFaceIcon, SIGNAL(pressed(QPoint)), 
            this, SLOT(slotFaceIconPressed(QPoint)));
    _currentFullName = new QLabel("FullName");
    formLayout->addRow(_currentFaceIcon, _currentFullName);

    _currentAccountType = new QComboBox;
    _currentAccountType->setMaximumWidth(124);
    _currentAccountType->addItem(i18n("Standard"));
    _currentAccountType->addItem(i18n("Administrator"));
    formLayout->addRow(i18n("Account Type"), _currentAccountType);

    _currentLanguage = new QLabel("English");
    // @pm not implement language, right?
    //formLayout->addRow(i18n("Language"), _currentLanguage);

    formLayout->addRow(new QLabel(i18n("Login Options")));

    _passwdEdit = new PwdEdit("password");
    _passwdEdit->setMaximumWidth(124);
    _passwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    connect(_passwdEdit, SIGNAL(pressed()), this, SLOT(slotPasswordEditPressed()));
    formLayout->addRow(i18n("Password"), _passwdEdit);

    hbox = new QHBoxLayout;
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

    _am = new QtAccountsService::AccountsManager;
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

void KCMUserAccount::slotFaceIconClicked(QString filePath) 
{
    if (_currentUser) {
        qDebug() << __PRETTY_FUNCTION__ << _currentUser->userName() << filePath;
        _currentUser->setIconFileName(filePath);
        _currentFaceIcon->setIcon(FaceIconPopup::faceIcon(filePath));
        _editUserName = _currentUser->userName();
        load();
    }
}

void KCMUserAccount::slotFaceIconPressed(QPoint pos)
{
    if (_currentUser->userName() != _ku->loginName()) {
        // Unlock

        return;
    }
    
    FaceIconPopup *faceIconPopup = new FaceIconPopup;
    connect(faceIconPopup, SIGNAL(clickFaceIcon(QString)), 
            this, SLOT(slotFaceIconClicked(QString)));
    faceIconPopup->popup(pos);
}

void KCMUserAccount::slotPasswordEditPressed() 
{
    ChgPwdDlg *chgPwdDlg = new ChgPwdDlg(_am, this);
    chgPwdDlg->show();
}

void KCMUserAccount::slotAddBtnClicked()
{
    AddUserDlg *addUserDlg = new AddUserDlg(_am, this);
    addUserDlg->show();
}

void KCMUserAccount::slotRemoveBtnClicked()
{
    RemoveUserDlg *removeUserDlg = new RemoveUserDlg(_am, this);
    removeUserDlg->show();
}

void KCMUserAccount::slotItemClicked(QListWidgetItem *item) 
{
    QString itemText = item->text();
    
    _currentUser = _am->findUserByName(itemText);

    if (_currentUser == NULL) {
        _removeBtn->setEnabled(false);
        return;
    }

    connect(_currentUser, SIGNAL(accountChanged()), this, SLOT(load()));

    _currentFaceIcon->setIcon(FaceIconPopup::faceIcon(_currentUser->iconFileName()));
    _currentFullName->setText(_currentUser->displayName());

    _currentAccountType->setCurrentText((int)_currentUser->accountType() ? 
                                        i18n("Administrator") : 
                                        i18n("Standard"));
    if (_currentUser->userName() == _ku->loginName()) {
        _currentAccountType->setEnabled(false);
        _removeBtn->setEnabled(false);
    } else {
        _currentAccountType->setEnabled(true);
        _removeBtn->setEnabled(true);
    }
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
    
    // My Account
    QString myAccountLoginName = _ku->loginName();
    
    QListWidgetItem *item = new QListWidgetItem(i18n("My Account"));
    _accountList->addItem(item);

    QtAccountsService::UserAccount *myUserAccount = _am->findUserByName(myAccountLoginName);
    QString iconFilePath = myUserAccount->iconFileName();
    item = new QListWidgetItem(FaceIconPopup::faceIcon(iconFilePath), 
                               myAccountLoginName);
    _accountList->addItem(item);
    _accountList->setCurrentItem(item);
    if (_editUserName == myAccountLoginName)
        _accountList->setCurrentItem(item);

    if (_editUserName == "")
        slotItemClicked(item);

    // Other Accounts
    item = new QListWidgetItem(i18n("Other Accounts"));
    _accountList->addItem(item);

    foreach (KUser user, _ku->allUsers()) {
        if (user.loginName() != myAccountLoginName && 
            user.homeDir().startsWith("/home")) {
            QtAccountsService::UserAccount *userAccount = 
                _am->findUserByName(user.loginName());
            QString iconFilePath = userAccount->iconFileName();
            item = new QListWidgetItem(
                        FaceIconPopup::faceIcon(iconFilePath), 
                        user.loginName());
            _accountList->addItem(item);

            if (_editUserName == user.loginName())
                _accountList->setCurrentItem(item);
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

    _currentUser->setAccountType((QtAccountsService::UserAccount::AccountType)_currentAccountType->currentIndex());

    // FIXME: QtAccountsService fail to setAutomaticLogin?
    _currentUser->setAutomaticLogin(_autoLoginButton->isChecked());
}

#include "main.moc"
