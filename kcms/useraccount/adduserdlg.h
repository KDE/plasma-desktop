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
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtAccountsService/AccountsManager>
#include <QtAccountsService/UserAccount>
#include <KDialog>

#include "faceiconbutton.h"
#include "faceiconpopup.h"

/* connect(_am, SIGNAL(userAdded(UserAccount *)),this, SLOT(slotUserAdded(UserAccount*)));
*/
typedef QtAccountsService::UserAccount UserAccount;

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
        QLineEdit *verifyPwdEdit;
        QLabel *verifyPwdLabel;
        QLabel *AccTypeLable;
        QLabel *autoLoginLabel;
        QCheckBox *autoLoginCheckBox;
        QCheckBox *canLoginCheckBox;
        QCheckBox *disLoginCheckBox;
        QLabel *canLoginLabel;
        QGroupBox *groupBox;
        QCheckBox *disLoginCheckBox_2;
        QCheckBox *disLoginCheckBox_3;
        QCheckBox *disLoginCheckBox_4;
        QLabel *canLoginLabel_2;
        FaceIconButton *faceIconPushButton;

    void setupUi(QWidget *Dialog)
    {
        if (Dialog->objectName().isEmpty())
            Dialog->setObjectName(QStringLiteral("Dialog"));

                Dialog->resize(400, 450);
                userNameEdit = new QLineEdit(Dialog);
                userNameEdit->setObjectName(QStringLiteral("userNameEdit"));
                userNameEdit->setGeometry(QRect(150, 81, 111, 21));
                userNameEdit->setContextMenuPolicy(Qt::NoContextMenu);
                userNameEdit->setAutoFillBackground(true);
                userNameEdit->setMaxLength(31);
                userNameEdit->setEchoMode(QLineEdit::Normal);
                QRegExp regx("^[a-zA-Z][A-Za-z0-9_-]+$");
                QValidator *validator = new QRegExpValidator(regx, userNameEdit );
                userNameEdit->setValidator( validator );


                userNameLabel = new QLabel(Dialog);
                userNameLabel->setObjectName(QStringLiteral("userNameLabel"));
                userNameLabel->setGeometry(QRect(10, 81, 131, 20));
                userNameLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                headLable = new QLabel(Dialog);
                headLable->setObjectName(QStringLiteral("headLable"));
                headLable->setGeometry(QRect(165, 20, 125, 23));
                QFont font;
                font.setPointSize(12);
                font.setBold(true);
                font.setWeight(75);
                headLable->setFont(font);
                headLable->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
                cancelBtn = new QPushButton(Dialog);
                cancelBtn->setObjectName(QStringLiteral("cancelBtn"));
                cancelBtn->setGeometry(QRect(240, 394, 75, 27));
                addBtn = new QPushButton(Dialog);
                addBtn->setObjectName(QStringLiteral("addBtn"));
                addBtn->setGeometry(QRect(80, 394, 75, 27));
                PwdEdit = new QLineEdit(Dialog);
                PwdEdit->setObjectName(QStringLiteral("PwdEdit"));
                PwdEdit->setGeometry(QRect(150, 121, 171, 21));
                PwdEdit->setMaxLength(256);
                PwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
                passwordLabel = new QLabel(Dialog);
                passwordLabel->setObjectName(QStringLiteral("passwordLabel"));
                passwordLabel->setGeometry(QRect(10, 121, 131, 20));
                passwordLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                verifyPwdEdit = new QLineEdit(Dialog);
                verifyPwdEdit->setObjectName(QStringLiteral("verifyPwdEdit"));
                verifyPwdEdit->setGeometry(QRect(150, 161, 171, 21));
                verifyPwdEdit->setMaxLength(256);
                verifyPwdEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
                verifyPwdLabel = new QLabel(Dialog);
                verifyPwdLabel->setObjectName(QStringLiteral("verifyPwdLabel"));
                verifyPwdLabel->setGeometry(QRect(10, 161, 131, 20));
                verifyPwdLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                AccTypeLable = new QLabel(Dialog);
                AccTypeLable->setObjectName(QStringLiteral("AccTypeLable"));
                AccTypeLable->setGeometry(QRect(10, 201, 131, 20));
                AccTypeLable->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                autoLoginLabel = new QLabel(Dialog);
                autoLoginLabel->setObjectName(QStringLiteral("autoLoginLabel"));
                autoLoginLabel->setGeometry(QRect(10, 241, 131, 20));
                autoLoginLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                autoLoginCheckBox = new QCheckBox(Dialog);
                autoLoginCheckBox->setObjectName(QStringLiteral("autoLoginCheckBox"));
                autoLoginCheckBox->setGeometry(QRect(150, 241, 51, 21));
                canLoginCheckBox = new QCheckBox(Dialog);
                canLoginCheckBox->setObjectName(QStringLiteral("canLoginCheckBox"));
                canLoginCheckBox->setGeometry(QRect(150, 201, 41, 21));
                disLoginCheckBox = new QCheckBox(Dialog);
                disLoginCheckBox->setObjectName(QStringLiteral("disLoginCheckBox"));
                disLoginCheckBox->setGeometry(QRect(150, 281, 51, 21));
                canLoginLabel = new QLabel(Dialog);
                canLoginLabel->setObjectName(QStringLiteral("canLoginLabel"));
                canLoginLabel->setGeometry(QRect(10, 281, 131, 20));
                canLoginLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                groupBox = new QGroupBox(Dialog);
                groupBox->setObjectName(QStringLiteral("groupBox"));
                groupBox->setGeometry(QRect(20, 323, 351, 41));
                disLoginCheckBox_2 = new QCheckBox(groupBox);
                disLoginCheckBox_2->setObjectName(QStringLiteral("disLoginCheckBox_2"));
                disLoginCheckBox_2->setGeometry(QRect(83, 10, 91, 21));
                QFont font1;
                font1.setPointSize(10);
                disLoginCheckBox_2->setFont(font1);
                disLoginCheckBox_3 = new QCheckBox(groupBox);
                disLoginCheckBox_3->setObjectName(QStringLiteral("disLoginCheckBox_3"));
                disLoginCheckBox_3->setGeometry(QRect(180, 10, 91, 21));
                disLoginCheckBox_3->setFont(font1);
                disLoginCheckBox_4 = new QCheckBox(groupBox);
                disLoginCheckBox_4->setObjectName(QStringLiteral("disLoginCheckBox_4"));
                disLoginCheckBox_4->setGeometry(QRect(280, 10, 61, 21));
                disLoginCheckBox_4->setFont(font1);
                canLoginLabel_2 = new QLabel(groupBox);
                canLoginLabel_2->setObjectName(QStringLiteral("canLoginLabel_2"));
                canLoginLabel_2->setGeometry(QRect(0, 10, 71, 21));
                canLoginLabel_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
                groupBox->setDisabled(true);

                faceIconPushButton = new FaceIconButton(Dialog);
                faceIconPushButton->setMinimumWidth(faceIconSize);
                faceIconPushButton->setMinimumHeight(faceIconSize);
                faceIconPushButton->setIconSize(QSize(faceIconSize, faceIconSize));
                faceIconPushButton->setObjectName(QStringLiteral("faceIconPushButton"));
                faceIconPushButton->setGeometry(QRect(275, 70, 46, 46));

                QWidget::setTabOrder(userNameEdit, faceIconPushButton);
                QWidget::setTabOrder(faceIconPushButton, PwdEdit);
                QWidget::setTabOrder(PwdEdit, verifyPwdEdit);
                QWidget::setTabOrder(verifyPwdEdit, canLoginCheckBox);
                QWidget::setTabOrder(canLoginCheckBox, autoLoginCheckBox);
                QWidget::setTabOrder(autoLoginCheckBox, disLoginCheckBox);
                QWidget::setTabOrder(disLoginCheckBox, disLoginCheckBox_2);
                QWidget::setTabOrder(disLoginCheckBox_2, disLoginCheckBox_3);
                QWidget::setTabOrder(disLoginCheckBox_3, disLoginCheckBox_4);
                QWidget::setTabOrder(disLoginCheckBox_4, addBtn);
                QWidget::setTabOrder(addBtn, cancelBtn);

            retranslateUi(Dialog);
            Dialog->setStyleSheet("QLineEdit{ background-color: rgb(202, 202, 202);border:1px solid #c0fff5;}");
            QMetaObject::connectSlotsByName(Dialog);
    } // setupUi

    void retranslateUi(QWidget *Dialog)
    {
            userNameLabel->setText(QApplication::translate("Dialog", "User Name", 0));
            headLable->setText(QApplication::translate("Dialog", "Add User", 0));
            cancelBtn->setText(QApplication::translate("Dialog", "Cancel", 0));
            addBtn->setText(QApplication::translate("Dialog", "Add", 0));
            passwordLabel->setText(QApplication::translate("Dialog", "Password", 0));
            verifyPwdLabel->setText(QApplication::translate("Dialog", "Verify Password", 0));
            AccTypeLable->setText(QApplication::translate("Dialog", "Administrator", 0));
            autoLoginLabel->setText(QApplication::translate("Dialog", "Automatic Login", 0));
            autoLoginCheckBox->setText(QString());
            canLoginCheckBox->setText(QString());
            disLoginCheckBox->setText(QString());
            canLoginLabel->setText(QApplication::translate("Dialog", "Login Not Permitted", 0));
            groupBox->setTitle(QString());
            disLoginCheckBox_2->setText(QApplication::translate("Dialog", "Password", 0));
            disLoginCheckBox_3->setText(QApplication::translate("Dialog", "Fingerprint", 0));
            disLoginCheckBox_4->setText(QApplication::translate("Dialog", "Iris", 0));
            canLoginLabel_2->setText(QApplication::translate("Dialog", "Login With:", 0));
    }

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
    void slotFaceIconClicked(QString filePath);
    void slotFaceIconPressed(QPoint pos);
    void slotUserAdded(UserAccount* ua);
private:
    QtAccountsService::AccountsManager *_am;
    Ui::Dialog ui;
    QWidget *Dlg =  NULL;
    FaceIconButton *_currentFaceIcon;
    QString iconFilePath;
};

#endif /* __ADDUSERDLG_H__ */
