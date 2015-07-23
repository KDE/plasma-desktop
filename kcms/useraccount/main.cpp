/**
 *  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *  Copyright (C) 2015 fjiang <fujiang.zhu@i-soft.com.cn>
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

#include <PolkitQt1/Authority>
#include <PolkitQt1/Subject>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QIcon>
#include <QMessageBox>

static const int opIconSize = 32;
static const QString unlockActionId = "org.freedesktop.accounts.user-administration";

K_PLUGIN_FACTORY(Factory,
        registerPlugin<KCMUserAccount>();
        )
K_EXPORT_PLUGIN(Factory("useraccount"))

KCMUserAccount::KCMUserAccount(QWidget *parent, const QVariantList &)
  : KCModule(parent),
    _accountList(NULL),
    _addBtn(NULL),
    _removeBtn(NULL),
    _ku(NULL),
    _am(NULL),
    _currentUser(NULL),
    _unlocked(false),
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
    _addBtn = new QPushButton;
    _addBtn->setEnabled(_unlocked);
    _addBtn->setMaximumWidth(opIconSize);
    _addBtn->setMaximumHeight(opIconSize);
    _addBtn->setIcon(QIcon::fromTheme("list-add"));
    connect(_addBtn, SIGNAL(clicked()), this, SLOT(slotAddBtnClicked()));
    hbox->addWidget(_addBtn);
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
    unlockBtn->setMaximumWidth(100);
    unlockBtn->setMaximumHeight(opIconSize);
    _actionBtn = new PolkitQt1::Gui::ActionButton(
        unlockBtn, unlockActionId, this);
    _actionBtn->setText(i18n("Unlock"));
    _actionBtn->setIcon(QIcon::fromTheme("document-encrypt"));
    connect(_actionBtn, SIGNAL(clicked(QAbstractButton *, bool)), 
            _actionBtn, SLOT(activate()));
    connect(_actionBtn, SIGNAL(authorized()), this, SLOT(actionActivated()));
    formLayout->addRow(unlockBtn);

    _currentFaceIcon = new FaceIconButton;
    _currentFaceIcon->setMinimumWidth(faceIconSize);
    _currentFaceIcon->setMinimumHeight(faceIconSize);
    _currentFaceIcon->setIconSize(QSize(faceIconSize, faceIconSize));
    connect(_currentFaceIcon, SIGNAL(pressed(QPoint)), 
            this, SLOT(slotFaceIconPressed(QPoint)));
    _currentFullName = new QLineEdit("FullName");
    _currentFullName->setEnabled(false);
    formLayout->addRow(_currentFaceIcon, _currentFullName);

    _currentAccountType = new QComboBox;
    _currentAccountType->setMaximumWidth(124);
    _currentAccountType->addItem(i18n("Standard"));
    _currentAccountType->addItem(i18n("Administrator"));
    formLayout->addRow(i18n("Account Type"), _currentAccountType);

    _currentLanguage = new QLabel("English");

    formLayout->addRow(new QLabel(i18n("Login Options")));

    _passwdEdit = new PwdEdit("password");
    _passwdEdit->setEnabled(false);
    _passwdEdit->setMaximumWidth(124);
    _passwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
    connect(_passwdEdit, SIGNAL(pressed()), this, SLOT(slotPasswordEditPressed()));
    formLayout->addRow(i18n("Password"), _passwdEdit);

    hbox = new QHBoxLayout;
    _autoLoginButton = new QRadioButton(i18n("Yes"));
    _autoLoginButton->setEnabled(_unlocked);
    _nonAutoLoginButton = new QRadioButton(i18n("No"));
    _nonAutoLoginButton->setEnabled(_unlocked);
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
    connect(_am, &QtAccountsService::AccountsManager::userDeleted,
            this, &KCMUserAccount::load);
    connect(_am, &QtAccountsService::AccountsManager::userAdded,
            this, &KCMUserAccount::load);
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

void KCMUserAccount::_unlockUi() 
{
    _actionBtn->setText(_unlocked ? i18n("Lock") : i18n("Unlock"));
    _actionBtn->setIcon(_unlocked ?
                        QIcon::fromTheme("document-encrypted") :
                        QIcon::fromTheme("document-encrypt"));
    _addBtn->setEnabled(_unlocked);
    _autoLoginButton->setEnabled(_unlocked);
    _nonAutoLoginButton->setEnabled(_unlocked);
    load();
}

void KCMUserAccount::slotUnlockBtnClicked() 
{
    if (_unlocked) {
        _unlocked = false;
        _unlockUi();
        disconnect(_actionBtn, SIGNAL(clicked(QAbstractButton *, bool)),
                   this, SLOT(slotUnlockBtnClicked()));
        connect(_actionBtn, SIGNAL(clicked(QAbstractButton *, bool)),
                _actionBtn, SLOT(activate()));
    }
}

void KCMUserAccount::slotFaceIconClicked(QString filePath) 
{
    if (_currentUser) {
        _currentUser->setIconFileName(filePath);
        _currentFaceIcon->setIcon(FaceIconPopup::faceIcon(filePath));
        _editUserName = _currentUser->userName();
        load();
    }
}

void KCMUserAccount::slotFaceIconPressed(QPoint pos)
{
    if (_currentUser->userName() != _ku->loginName()) {
        if (!_unlocked)
            return;
    }
    
    FaceIconPopup *faceIconPopup = new FaceIconPopup;
    connect(faceIconPopup, SIGNAL(clickFaceIcon(QString)), 
            this, SLOT(slotFaceIconClicked(QString)));
    faceIconPopup->popup(pos);
}

void KCMUserAccount::slotPasswordEditPressed() 
{
    ChgPwdDlg *chgPwdDlg = new ChgPwdDlg(_currentUser, this);
    chgPwdDlg->show();

    if (chgPwdDlg->exec() == QDialog::Accepted)
        QMessageBox::warning(this, "warning", "info", "okok");

    delete chgPwdDlg;
}

void KCMUserAccount::slotAddBtnClicked()
{
    AddUserDlg *addUserDlg = new AddUserDlg(_am, this);
    addUserDlg->show();
}

void KCMUserAccount::slotRemoveBtnClicked()
{
    char info[512] = "";

    if (!_currentUser || !_am)
        return;

    if (getuid() == _currentUser->userId()) {
        strncpy(info, "You cannot delete your own account.", sizeof(info));
        QMessageBox::warning(this, "warning", info);
        return;
    } else if(0) {
        strncpy(info, "Deleting a user while they are logged in can leave the "
                      "system in an inconsistent state.", 
                sizeof(info));
        QMessageBox::warning(this, "warning", info);
        return;
    } else if(_currentUser->isLocalAccount()) {
        snprintf(info, sizeof(info) - 1, "will delete local account, uid[%d]", 
                 (int)_currentUser->userId());
    } else {
        strncpy(info, "will delete enterprise user account.", sizeof(info));
        QMessageBox::warning(this, "warning", info);
        return;
    }

    RemoveUserDlg *removeUserDlg = new RemoveUserDlg(_am, _currentUser, this);
    if (removeUserDlg->exec() == QDialog::Accepted)
        QMessageBox::warning(this, "warning", "info", "ok");

    delete removeUserDlg;

    return;
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
        _currentFullName->setEnabled(true);
        _currentAccountType->setEnabled(false);
        _passwdEdit->setEnabled(true);
        _removeBtn->setEnabled(false);
    } else {
        _currentFullName->setEnabled(_unlocked);
        _currentAccountType->setEnabled(_unlocked);
        _passwdEdit->setEnabled(_unlocked);
        _removeBtn->setEnabled(_unlocked);
    }
    connect(_currentFullName, SIGNAL(editingFinished()), this, SLOT(changed()));
    connect(_currentAccountType, SIGNAL(activated(int)), this, SLOT(changed()));

    _currentLanguage->setText(_currentUser->language());

    connect(_passwdEdit, SIGNAL(textChanged(const QString &)), this, SLOT(changed()));

    _autoLoginButton->setChecked(_currentUser->automaticLogin());
    connect(_autoLoginButton, SIGNAL(clicked()), this, SLOT(changed()));
    _nonAutoLoginButton->setChecked(!_currentUser->automaticLogin());
    connect(_nonAutoLoginButton, SIGNAL(clicked()), this, SLOT(changed()));
}

void KCMUserAccount::actionActivated()
{
    PolkitQt1::Authority::Result result;
    PolkitQt1::UnixProcessSubject subject(static_cast<uint>(
        QCoreApplication::applicationPid()));

    result = PolkitQt1::Authority::instance()->checkAuthorizationSync(
        unlockActionId, subject, PolkitQt1::Authority::AllowUserInteraction);
    if (result == PolkitQt1::Authority::Yes)
        _unlocked = true;
    else 
        _unlocked = false;

    _unlockUi();
    disconnect(_actionBtn, SIGNAL(clicked(QAbstractButton *, bool)),
               _actionBtn, SLOT(activate()));
    connect(_actionBtn, SIGNAL(clicked(QAbstractButton*, bool)),
            this, SLOT(slotUnlockBtnClicked()));
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

    if (_currentUser == NULL)
        return;

    _currentUser->setRealName(_currentFullName->text());

    _currentUser->setAccountType((QtAccountsService::UserAccount::AccountType)_currentAccountType->currentIndex());

    _currentUser->setAutomaticLogin(_autoLoginButton->isChecked());
}

#include "main.moc"
