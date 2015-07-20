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

#ifndef MAIN_H
#define MAIN_H

#include <kcmodule.h>

/* QtAccountsService */
#include <QtAccountsService/AccountsManager>

#include "faceiconbutton.h"
#include "pwdedit.h"

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>

class KUser;
class QObject;

class KCMUserAccount : public KCModule
{
	Q_OBJECT

public:
	explicit KCMUserAccount(QWidget *parent, const QVariantList & list = QVariantList());
	~KCMUserAccount();

	void load();
    void save();

private slots:
    void slotItemClicked(QListWidgetItem *item);
    void slotFaceIconClicked(QString filePath);
    void slotFaceIconPressed(QPoint pos);
    void slotPasswordEditPressed();
    void slotAddBtnClicked();
    void slotRemoveBtnClicked();

private:
    KUser *_ku;
    QtAccountsService::AccountsManager *_am;
    QtAccountsService::UserAccount *_currentUser;
    QListWidget *_accountList;
    QPushButton *_removeBtn;
    FaceIconButton *_currentFaceIcon;
    QLabel *_currentFullName;
    QComboBox *_currentAccountType;
    QLabel *_currentLanguage;
    PwdEdit *_passwdEdit;
    QRadioButton *_autoLoginButton;
    QRadioButton *_nonAutoLoginButton;
    QString _editUserName;
    QString _editIconFilePath;

    QIcon _faceIcon(QString faceIconPath);
    QPixmap _facePixmap(QString faceIconPath);
};

#endif // MAIN_H
