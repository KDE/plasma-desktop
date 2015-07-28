/**
 *  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
 *  Copyright (C) 2015 fjiang <fujiang.zhu@i-soft.com.cn>
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

#ifndef __ADDUSERDLG_H__
#define __ADDUSERDLG_H__

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtAccountsService/AccountsManager>
#include <KDialog>

class Ui_Dialog
{
public:
    QLineEdit *userNameEdit;
    QLabel *userNameLabel;
    QLabel *headLable;
    QPushButton *cancelBtn;
    QPushButton *addBtn;
    QLineEdit *PwdEdit;
    QLabel *passwordLabel;
    QLabel *littleHeadLabel;
    QLineEdit *verifyPwdEdit;
    QLabel *verifyPwdLabel;
    QLabel *AccTypeLable;
    QComboBox *AccTypeComBox;
    QLabel *autoLoginLabel;
    QCheckBox *autoLoginCheckBox;
    QCheckBox *canLoginCheckBox;
    QLabel *canLoginLabel;

    void setupUi(QWidget *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

		Dialog->resize(436, 460);
        userNameEdit = new QLineEdit(Dialog);
        userNameEdit->setObjectName(QStringLiteral("userNameEdit"));
        userNameEdit->setGeometry(QRect(150, 113, 171, 21));
        userNameEdit->setContextMenuPolicy(Qt::NoContextMenu);
        userNameEdit->setAutoFillBackground(true);
        userNameEdit->setMaxLength(32);
        userNameEdit->setEchoMode(QLineEdit::Normal);
        QRegExp regx("[A-Za-z0-9_-]+$");
        QValidator *validator = new QRegExpValidator(regx, userNameEdit );
        userNameEdit->setValidator( validator );
        userNameLabel = new QLabel(Dialog);
        userNameLabel->setObjectName(QStringLiteral("userNameLabel"));
        userNameLabel->setGeometry(QRect(10, 113, 131, 20));
        userNameLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        headLable = new QLabel(Dialog);
        headLable->setObjectName(QStringLiteral("headLable"));
        headLable->setGeometry(QRect(27, 24, 125, 23));
        QFont font;
        font.setPointSize(12);
        font.setBold(true);
        font.setWeight(75);
        headLable->setFont(font);
        headLable->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        cancelBtn = new QPushButton(Dialog);
        cancelBtn->setObjectName(QStringLiteral("cancelBtn"));
        cancelBtn->setGeometry(QRect(260, 394, 75, 27));
        addBtn = new QPushButton(Dialog);
        addBtn->setObjectName(QStringLiteral("addBtn"));
        addBtn->setGeometry(QRect(100, 394, 75, 27));
        PwdEdit = new QLineEdit(Dialog);
        PwdEdit->setObjectName(QStringLiteral("PwdEdit"));
        PwdEdit->setGeometry(QRect(150, 153, 171, 21));
        PwdEdit->setMaxLength(32);
        PwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        passwordLabel = new QLabel(Dialog);
        passwordLabel->setObjectName(QStringLiteral("passwordLabel"));
        passwordLabel->setGeometry(QRect(10, 153, 131, 20));
        passwordLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        littleHeadLabel = new QLabel(Dialog);
        littleHeadLabel->setObjectName(QStringLiteral("littleHeadLabel"));
        littleHeadLabel->setGeometry(QRect(27, 55, 141, 21));
        littleHeadLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        verifyPwdEdit = new QLineEdit(Dialog);
        verifyPwdEdit->setObjectName(QStringLiteral("verifyPwdEdit"));
        verifyPwdEdit->setGeometry(QRect(150, 193, 171, 21));
        verifyPwdEdit->setMaxLength(32);
        verifyPwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
        verifyPwdLabel = new QLabel(Dialog);
        verifyPwdLabel->setObjectName(QStringLiteral("verifyPwdLabel"));
        verifyPwdLabel->setGeometry(QRect(10, 193, 131, 20));
        verifyPwdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        AccTypeLable = new QLabel(Dialog);
        AccTypeLable->setObjectName(QStringLiteral("AccTypeLable"));
        AccTypeLable->setGeometry(QRect(10, 236, 131, 20));
        AccTypeLable->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        AccTypeComBox = new QComboBox(Dialog);
        AccTypeComBox->setObjectName(QStringLiteral("AccTypeComBox"));
        AccTypeComBox->setGeometry(QRect(150, 233, 171, 25));
        autoLoginLabel = new QLabel(Dialog);
        autoLoginLabel->setObjectName(QStringLiteral("autoLoginLabel"));
        autoLoginLabel->setGeometry(QRect(10, 273, 131, 20));
        autoLoginLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
        autoLoginCheckBox = new QCheckBox(Dialog);
        autoLoginCheckBox->setObjectName(QStringLiteral("autoLoginCheckBox"));
        autoLoginCheckBox->setGeometry(QRect(150, 273, 91, 21));
        canLoginCheckBox = new QCheckBox(Dialog);
        canLoginCheckBox->setObjectName(QStringLiteral("canLoginCheckBox"));
        canLoginCheckBox->setGeometry(QRect(150, 305, 91, 21));
        canLoginLabel = new QLabel(Dialog);
        canLoginLabel->setObjectName(QStringLiteral("canLoginLabel"));
        canLoginLabel->setGeometry(QRect(10, 305, 131, 20));
        canLoginLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        retranslateUi(Dialog);
        Dialog->setStyleSheet("QLineEdit{ background-color: rgb(202, 202, 202);border:1px solid #c0fff5;}");
        QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QWidget *Dialog)
    {
        Dialog->setWindowTitle(QApplication::translate("Dialog", "Dialog", 0));
        userNameLabel->setText(QApplication::translate("Dialog", "User Name", 0));
        headLable->setText(QApplication::translate("Dialog", "Add User", 0));
        cancelBtn->setText(QApplication::translate("Dialog", "Cancel", 0));
        addBtn->setText(QApplication::translate("Dialog", "Add", 0));
        passwordLabel->setText(QApplication::translate("Dialog", "Password", 0));
        littleHeadLabel->setText(QApplication::translate("Dialog", "Add New User", 0));
        verifyPwdLabel->setText(QApplication::translate("Dialog", "Verify Password", 0));
        AccTypeLable->setText(QApplication::translate("Dialog", "Account Type", 0));
        AccTypeComBox->clear();
        AccTypeComBox->insertItems(0, QStringList()
         << QApplication::translate("Dialog", "Standard", 0)
         << QApplication::translate("Dialog", "Administrator", 0)
        );
        autoLoginLabel->setText(QApplication::translate("Dialog", "Auto Login", 0));
        autoLoginCheckBox->setText(QApplication::translate("Dialog", "Enabled", 0));
        canLoginCheckBox->setText(QApplication::translate("Dialog", "Enabled", 0));
        canLoginLabel->setText(QApplication::translate("Dialog", "Login Not Permitted", 0));
    } // retranslateUi

};

namespace Ui {
    class Dialog: public Ui_Dialog {};
} // namespace Ui



class AddUserDlg : public KDialog
{
    Q_OBJECT

public:
    explicit AddUserDlg(QtAccountsService::AccountsManager *am, 
                        QWidget *parent = NULL, 
                        Qt::WindowFlags f = Qt::Dialog);
    ~AddUserDlg();

private slots:
    void slotAddUser();

private:
    QtAccountsService::AccountsManager *_am;
    Ui::Dialog ui;
    QWidget *Dlg =  NULL;
};

#endif /* __ADDUSERDLG_H__ */
