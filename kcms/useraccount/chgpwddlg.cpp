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
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);

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
    Dlg->setWindowModality(Qt::WindowModal);

    QObject::connect(ui.cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui.changeBtn, SIGNAL(clicked()), this, SLOT(slotChangePwd()));

    setFixedSize(515, 250);
}

ChgPwdDlg::~ChgPwdDlg() 
{

    if (ui.curPwdEdit) delete ui.curPwdEdit;
    if (ui.curPwdLabel) delete ui.curPwdLabel;
    if (ui.curPwdLabel_2) delete ui.curPwdLabel_2;
    if (ui.cancelBtn) delete ui.cancelBtn;
    if (ui.changeBtn) delete ui.changeBtn;
    if (ui.newPwdEdit) delete ui.newPwdEdit;
    if (ui.verPwdEdit) delete ui.verPwdEdit;
    if (ui.verPwdLabel) delete ui.verPwdLabel;
    if (ui.newPwdLabel) delete ui.newPwdLabel;

    delete Dlg;
}

void ChgPwdDlg::slotChangePwd()
{
    if (!ui.newPwdEdit || !ui.verPwdEdit)
        close();

    if (!_ua)
        close();

    if (ui.newPwdEdit->text().isEmpty() || ui.verPwdEdit->text().isEmpty())
        return;

    QString newStr = ui.newPwdEdit->text().trimmed();
    QString verStr = ui.verPwdEdit->text().trimmed();

    if (newStr == verStr) {
        qsrand(time(NULL));
        int n = qrand();
        char salt[8] = "";
        snprintf(salt, sizeof(salt), "$6$%02d", n);
        char *crystr = crypt(qPrintable(newStr), salt);
        if (crystr == NULL) {
            QMessageBox::warning(this, "warning", 
                i18n("fail to crypt password, please try again."));
            close();
        }
        _ua->setPassword(crystr);
        KWallet::Wallet::changePassword(newStr, effectiveWinId());
        close();
    } else {
        QMessageBox::warning(this, "warning", 
            i18n("password are not the same, please input again."));
    }

    return;
}
