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

#include "chgpwddlg.h"

#include <unistd.h>
#include <QMessageBox>
#include <kwallet.h>
#include <kdialog.h>

ChgPwdDlg::ChgPwdDlg(QtAccountsService::UserAccount *ua,
                     QWidget *parent, 
                     Qt::WindowFlags f)
  : KDialog(parent, f),
    _ua(ua)
{
    setWindowModality(Qt::WindowModal);
    setButtons(0);

    Dlg = new QWidget;
    ui.setupUi(Dlg);

    if (ua) {
        if (ua->accountType() == QtAccountsService::UserAccount::AdministratorAccountType) {
            ui.curPwdLabel->hide();
            ui.curPwdEdit->hide();
            ui.newPwdEdit->setGeometry(QRect(180, 80, 271, 21));
            ui.newPwdLabel->setGeometry(QRect(20, 80, 141, 20));
        }
    }
    setMainWidget(Dlg);
    this->setWindowTitle(i18n("Change Password"));

    QObject::connect(ui.cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui.changeBtn, SIGNAL(clicked()), this, SLOT(slotChangePwd()));

    setFixedSize(515, 250);
}

ChgPwdDlg::~ChgPwdDlg() 
{
    delete Dlg;
}

void ChgPwdDlg::slotChangePwd()
{
    if (!ui.newPwdEdit || !ui.verPwdEdit) {
        return;
    }

    if (!_ua) {
        close();
        return;
    }

    if (ui.newPwdEdit->text().isEmpty() || ui.verPwdEdit->text().isEmpty())
        return;

    QString newStr = ui.newPwdEdit->text().trimmed();
    QString verStr = ui.verPwdEdit->text().trimmed();

    if (newStr == verStr) {
        qsrand(time(NULL));
        int n = qrand();
        char salt[8] = "";
        snprintf(salt, sizeof(salt) - 1, "$6$%02d", n);
        char *crystr = crypt(qPrintable(newStr), salt);
        if (crystr == NULL) {
            QMessageBox::warning(this, "warning",
                i18n("fail to crypt password, please try again."));
            close();
        }
        _ua->setPassword(crystr);
        // TODO:
        //KWallet::Wallet::changePassword(_ua->userName(), effectiveWinId());
        close();
    } else {
        QMessageBox::warning(this, "warning", 
            i18n("password are not the same, please input again."));
    }

    return;
}
