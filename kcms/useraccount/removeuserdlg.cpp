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

#include "removeuserdlg.h"

#include <unistd.h>
#include <QMessageBox>
#include <qapplication.h>
#include <kdialog.h>

RemoveUserDlg::RemoveUserDlg(QtAccountsService::AccountsManager *am,
                             QtAccountsService::UserAccount *ua,
                             QWidget *parent, 
                             Qt::WindowFlags f)
  : KDialog(parent),
    _am(am),
    _ua(ua)
{
    setWindowModality(Qt::WindowModal);
    setButtons(0);
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);

    Dlg = new QWidget;
    ui.setupUi(Dlg);

    setMainWidget(Dlg);
    Dlg->setWindowModality(Qt::WindowModal);

    QObject::connect(ui._cancelBtn, SIGNAL(clicked()), this, SLOT(close()));
    QObject::connect(ui._delBtn, SIGNAL(clicked()), this, SLOT(slotDeleteByAction()));
    QObject::connect(ui._delAccountRadio, SIGNAL(clicked()), this, SLOT(slotDeleteUserKeepFile()));
    QObject::connect(ui._delAllRadio, SIGNAL(clicked()), this, SLOT(slotDeleteUserDelFile()));
    QObject::connect(ui._delBakRadio, SIGNAL(clicked()), this, SLOT(slotDeleteUserMoveFile()));
    setFixedSize(515, 250);
}


void RemoveUserDlg::relDeleteUser(bool keepFileFlag)
{
    if (_ua && _ua->automaticLogin())
        _ua->setAutomaticLogin(0);

    if (_am && _am->deleteUser(_ua->userId(), !keepFileFlag)) {
        _am->uncacheUser(_ua);
    } else {
        char info[128] = "";
        strncpy(info, "Failed to delete user", sizeof(info));
        QMessageBox::warning(this, "warning", info);
    }

    close();
}


void RemoveUserDlg::slotDeleteByAction()
{
    if (keepFileFlag == 1)
        relDeleteUser(true);
    else if (keepFileFlag == 0)
        relDeleteUser(false);
    else if (keepFileFlag == 2)
        close();
    else
        close();
}

void RemoveUserDlg::slotDeleteUserDelFile()
{
    keepFileFlag = 0;
}


void RemoveUserDlg::slotDeleteUserKeepFile()
{
    keepFileFlag = 1;
}

void RemoveUserDlg::slotDeleteUserMoveFile()
{
    keepFileFlag = 2;
    return;
}

RemoveUserDlg::~RemoveUserDlg() 
{
    delete Dlg;
}
