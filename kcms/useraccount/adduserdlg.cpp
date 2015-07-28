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

#include "adduserdlg.h"

#include <unistd.h>
#include <QMessageBox>
#include <KLocalizedString>
#include <QtAccountsService/AccountsManager>
#include <QtAccountsService/UserAccount>

AddUserDlg::AddUserDlg(QtAccountsService::AccountsManager *am, 
                       QWidget *parent, 
                       Qt::WindowFlags f)
  : KDialog(parent, f),
    _am(am)
{
    setWindowModality(Qt::WindowModal);
    setButtons(0);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);

    Dlg = new QWidget;
    ui.setupUi(Dlg);

    setMainWidget(Dlg);
    Dlg->setWindowModality(Qt::WindowModal);

    QObject::connect(ui.cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui.addBtn, SIGNAL(clicked()), this, SLOT(slotAddUser()));

    setFixedSize(436, 460);
}

AddUserDlg::~AddUserDlg() 
{
    if (Dlg) {
        delete Dlg;
    }
}

void AddUserDlg::slotAddUser()
{
    if (!_am) {
        close();
    }

    QString userName = ui.userNameEdit->text().trimmed();

    if (userName.isEmpty()) {
        QMessageBox::warning(this, "warning",
            i18n("Please input user name."));
        return;
    }

    if (ui.PwdEdit->text().isEmpty()) {
        QMessageBox::warning(this, "warning",
            i18n("Please input  account password."));
        return;
    }

    if (ui.verifyPwdEdit->text().isEmpty()) {
        QMessageBox::warning(this, "warning",
            i18n("Please input  verify password."));
        return;
    }

    if (ui.PwdEdit->text() != ui.verifyPwdEdit->text()) {
        QMessageBox::warning(this, "warning",
            i18n("Passwords do not match."));
        return;
    }

    QtAccountsService::UserAccount *user = _am->findUserByName(userName);

    if (user) {
        QMessageBox::warning(this, "warning",i18n("The user already exists"));
        return;
    }

    QtAccountsService::UserAccount::AccountType accType = ui.AccTypeComBox->currentIndex() == 0 ?
                QtAccountsService::UserAccount::StandardAccountType :
                QtAccountsService::UserAccount::AdministratorAccountType;

    bool ret = _am->createUser(userName,userName,accType);
    if (!ret) {
        QMessageBox::warning(this, "warning",i18n("Failed to add user."));
        return;
    }

    QTime dieTime = QTime::currentTime().addSecs(1);

    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);

    QtAccountsService::AccountsManager *__am = new QtAccountsService::AccountsManager;
    user = __am->findUserByName(userName);

    if (!user) {
        QMessageBox::warning(this, "warning",
            i18n("The user has been added,but cannot find it, please change password manually."));
        delete __am;
        close();
        return;
    }

    qsrand(time(NULL));
    int n = qrand();
    char salt[8] = "";
    snprintf(salt, sizeof(salt) - 1, "$6$%02d", n);
    char *crystr = crypt(qPrintable(ui.verifyPwdEdit->text()), salt);
    if (crystr == NULL) {
        QMessageBox::warning(this, "warning",
            i18n("fail to crypt password, please change password manually."));
        delete __am;
        close();
        return;
    }
    user->setPassword(crystr);
    user->setAutomaticLogin(ui.autoLoginCheckBox->isChecked()?true:false);

    delete __am;

    close();
}
