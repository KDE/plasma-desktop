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

#ifndef __CHGPWDDLG_H__
#define __CHGPWDDLG_H__

#include <KLocalizedString>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtAccountsService/AccountsManager>
#include <QtGui>
#include <QtWidgets/QMainWindow>
#include <KDialog>
#include <QtWidgets/QGridLayout>

class Ui_pwdDlg
{
public:
    QLineEdit *curPwdEdit = NULL;
    QLabel *curPwdLabel = NULL;
    QLabel *curPwdLabel_2 = NULL;
    QPushButton *cancelBtn = NULL;
    QPushButton *changeBtn = NULL;
    QLineEdit *newPwdEdit = NULL;
    QLineEdit *verPwdEdit = NULL;
    QLabel *verPwdLabel = NULL;
    QLabel *newPwdLabel = NULL;

    void setupUi(QWidget *Dialog)
    {
        if (Dialog->objectName().isEmpty()) {
            Dialog->setObjectName(QStringLiteral("chgpwdDlg"));

            Dialog->resize(515, 241);
            curPwdEdit = new QLineEdit(Dialog);
            curPwdEdit->setObjectName(QStringLiteral("curPwdEdit"));
            curPwdEdit->setGeometry(QRect(180, 70, 271, 21));
            curPwdEdit->setMaxLength(32);
            curPwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
            curPwdEdit->setAutoFillBackground(true);
            curPwdLabel = new QLabel(Dialog);
            curPwdLabel->setObjectName(QStringLiteral("curPwdLabel"));
            curPwdLabel->setGeometry(QRect(20, 70, 141, 20));
            curPwdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
            curPwdLabel_2 = new QLabel(Dialog);
            curPwdLabel_2->setObjectName(QStringLiteral("curPwdLabel_2"));
            curPwdLabel_2->setGeometry(QRect(180, 10, 125, 23));
            QFont font;
            font.setPointSize(12);
            font.setBold(true);
            font.setWeight(75);
            curPwdLabel_2->setFont(font);
            curPwdLabel_2->setAlignment(Qt::AlignCenter);
            cancelBtn = new QPushButton(Dialog);
            cancelBtn->setObjectName(QStringLiteral("cancelBtn"));
            cancelBtn->setGeometry(QRect(300, 200, 75, 27));
            changeBtn = new QPushButton(Dialog);
            changeBtn->setObjectName(QStringLiteral("changeBtn"));
            changeBtn->setGeometry(QRect(140, 200, 75, 27));
            newPwdEdit = new QLineEdit(Dialog);
            newPwdEdit->setObjectName(QStringLiteral("newPwdEdit"));
            newPwdEdit->setGeometry(QRect(180, 110, 271, 21));
            newPwdEdit->setMaxLength(32);
            newPwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
            verPwdEdit = new QLineEdit(Dialog);
            verPwdEdit->setObjectName(QStringLiteral("verPwdEdit"));
            verPwdEdit->setGeometry(QRect(180, 150, 271, 21));
            verPwdEdit->setMaxLength(32);
            verPwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
            verPwdLabel = new QLabel(Dialog);
            verPwdLabel->setObjectName(QStringLiteral("verPwdLabel"));
            verPwdLabel->setGeometry(QRect(20, 150, 141, 20));
            verPwdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
            newPwdLabel = new QLabel(Dialog);
            newPwdLabel->setObjectName(QStringLiteral("newPwdLabel"));
            newPwdLabel->setGeometry(QRect(20, 110, 141, 20));
            newPwdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

            Dialog->setStyleSheet("QLineEdit{ background-color: rgb(202, 202, 202);border:1px solid #c0fff5;}");
            retranslateUi(Dialog);
        }
    }

    void retranslateUi(QWidget *dlg)
    {
        curPwdLabel->setText(i18n("Current Password"));
        newPwdLabel->setText(i18n("New Password"));
        verPwdLabel->setText(i18n("Verify New Password"));
        cancelBtn->setText(i18n("cancel"));
        changeBtn->setText(i18n("change"));
        curPwdLabel_2->setText(i18n("change password"));
    }

};

namespace Ui {
    class pwdDlg: public Ui_pwdDlg {};
}

class ChgPwdDlg : public KDialog
{
    Q_OBJECT

public:
    explicit ChgPwdDlg(QtAccountsService::UserAccount *ua,
                       QWidget *parent = NULL, 
                       Qt::WindowFlags f = Qt::Dialog);

    QWidget *Dlg;

    ~ChgPwdDlg();

private slots:
    void slotChangePwd();

private:
    QtAccountsService::UserAccount *_ua;
    Ui::pwdDlg ui;
};

#endif /* __CHGPWDDLG_H__ */
