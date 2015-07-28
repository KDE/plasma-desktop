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

#ifndef __REMOVEUSERDLG_H__
#define __REMOVEUSERDLG_H__

#include <stdbool.h>
#include <KLocalizedString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtAccountsService/AccountsManager>
#include <QtGui>
#include <QtWidgets/QMainWindow>
#include <KDialog>
#include <QtWidgets/QGridLayout>
#include <QRadioButton>

class Ui_dlg
{
public:
    QLabel *_deleteInfoHead;
    QRadioButton *_delAllRadio;
    QRadioButton *_delAccountRadio;
    QRadioButton *_delBakRadio;
    QPushButton *_delBtn;
    QPushButton *_cancelBtn;
    QLabel *_deleteInfo;

    void setupUi(QWidget *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("delDlg"));

        Dialog->resize(515, 241);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(Dialog->sizePolicy().hasHeightForWidth());
        Dialog->setSizePolicy(sizePolicy);
        _deleteInfoHead = new QLabel(Dialog);
        _deleteInfoHead->setObjectName(QStringLiteral("_deleteInfoHead"));
        _deleteInfoHead->setGeometry(QRect(75, 25, 161, 16));
        QFont font;
        font.setPointSize(13);
        font.setBold(true);
        font.setWeight(75);
        _deleteInfoHead->setFont(font);
        _deleteInfoHead->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        _deleteInfoHead->setWordWrap(false);
        _delAllRadio = new QRadioButton(Dialog);
        _delAllRadio->setObjectName(QStringLiteral("_delAllRadio"));
        _delAllRadio->setGeometry(QRect(93, 89, 200, 21));
        _delAllRadio->setChecked(true);
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(_delAllRadio->sizePolicy().hasHeightForWidth());
        _delAllRadio->setSizePolicy(sizePolicy1);
        _delAccountRadio = new QRadioButton(Dialog);
        _delAccountRadio->setObjectName(QStringLiteral("_delAccountRadio"));
        _delAccountRadio->setGeometry(QRect(93, 119, 191, 21));
        sizePolicy1.setHeightForWidth(_delAccountRadio->sizePolicy().hasHeightForWidth());
        _delAccountRadio->setSizePolicy(sizePolicy1);
        _delBakRadio = new QRadioButton(Dialog);
        _delBakRadio->setObjectName(QStringLiteral("_delBakRadio"));
        _delBakRadio->setGeometry(QRect(93, 149, 381, 21));
        sizePolicy1.setHeightForWidth(_delBakRadio->sizePolicy().hasHeightForWidth());
        _delBakRadio->setSizePolicy(sizePolicy1);
        _delBtn = new QPushButton(Dialog);
        _delBtn->setObjectName(QStringLiteral("_delBtn"));
        _delBtn->setGeometry(QRect(151, 200, 98, 27));
        _cancelBtn = new QPushButton(Dialog);
        _cancelBtn->setObjectName(QStringLiteral("_cancelBtn"));
        _cancelBtn->setGeometry(QRect(289, 200, 98, 27));
        _deleteInfo = new QLabel(Dialog);
        _deleteInfo->setObjectName(QStringLiteral("_deleteInfo"));
        _deleteInfo->setGeometry(QRect(75, 56, 151, 16));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(_deleteInfo->sizePolicy().hasHeightForWidth());
        _deleteInfo->setSizePolicy(sizePolicy2);
        QFont font1;
        font1.setPointSize(10);
        font1.setBold(false);
        font1.setWeight(50);
        font1.setKerning(true);
        _deleteInfo->setFont(font1);
        _deleteInfo->setScaledContents(true);
        _deleteInfo->setWordWrap(true);

        retranslateUi(Dialog);
    }

    void retranslateUi(QWidget *Dialog)
    {
        _deleteInfoHead->setText(i18n("Delete User Account"));
        _delAllRadio->setText(i18n("Delete User Account and Files"));
        _delAccountRadio->setText(i18n("Delete User Account"));
        _delBakRadio->setText(i18n("Backup User Account Files Before Delete User Account"));
        _delBtn->setText(i18n("Ok"));
        _cancelBtn->setText(i18n("Cancel"));
        _deleteInfo->setText(i18n("Delete User Account"));
    }

};

namespace Ui 
{
    class dlg: public Ui_dlg {};
}


class RemoveUserDlg : public KDialog
{
    Q_OBJECT

public:
    explicit RemoveUserDlg(QtAccountsService::AccountsManager *am,
                           QtAccountsService::UserAccount *ua,
                           QWidget *parent = NULL, 
                           Qt::WindowFlags f = Qt::Dialog);
    ~RemoveUserDlg();

private slots:
    void slotDeleteUserKeepFile();
    void slotDeleteUserDelFile();
    void slotDeleteUserMoveFile();
    void slotDeleteByAction();

private:
    QtAccountsService::AccountsManager *_am;
    QtAccountsService::UserAccount *_ua;

    Ui::dlg ui;
    QWidget *Dlg;
    void relDeleteUser(bool keepFileFlag);
    int keepFileFlag = 0;
};



#endif /* __REMOVEUSERDLG_H__ */
