/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>
    Copyright (c) 2009 Dmitry Suzdalev <dimsuz@gmail.com>
    Copyright (c) 2010 Frederik Gladhorn <gladhorn@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "providerconfigwidget.h"

#include <KDebug>
#include <KMessageBox>
#include <KColorScheme>
#include <kicon.h>

#include <attica/person.h>

static const int loginTabIdx = 0;
static const int registerTabIdx = 1;

ProviderConfigWidget::ProviderConfigWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
}

void ProviderConfigWidget::setProvider(const Attica::Provider& provider)
{
    m_provider = provider;

    // TODO ensure that it reinits all fields nicely for new provider!
    initLoginPage();
    initRegisterPage();

    m_ui.userEditLP->setFocus();
}

void ProviderConfigWidget::initLoginPage()
{
    QString header;
    if (m_provider.name().isEmpty()) {
        header = i18n("Account details");
    } else {
        header = i18n("Account details for %1", m_provider.name());
    }
    m_ui.titleWidgetLogin->setText(header);
    m_ui.tabWidget->setTabIcon(loginTabIdx, KIcon("applications-internet"));

    if (m_provider.hasCredentials()) {
        QString user;
        QString password;
        m_provider.loadCredentials(user, password);
        kDebug() << "cred for: " << user;
        m_ui.userEditLP->setText(user);
        m_ui.passwordEditLP->setText(password);
    } else {
        m_ui.userEditLP->clear();
        m_ui.passwordEditLP->clear();
    }
    m_ui.enableProviderCheckBox->setChecked(m_provider.isEnabled());
    m_ui.testLoginButton->setIcon(KIcon("network-connect"));
    m_ui.iconLabelLP->setPixmap(KIcon("help-about").pixmap(24,24));

    connect(m_ui.userEditLP, SIGNAL(textChanged(const QString&)), this, SLOT(onLoginChanged()));
    connect(m_ui.passwordEditLP, SIGNAL(textChanged(const QString&)), this, SLOT(onLoginChanged()));
    connect(m_ui.testLoginButton, SIGNAL(clicked()), this, SLOT(onTestLogin()));
    connect(m_ui.infoLabelLP, SIGNAL(linkActivated(const QString&)), this, SLOT(onInfoLinkActivated()));
    connect(m_ui.enableProviderCheckBox, SIGNAL(clicked(bool)), this, SLOT(enableToggled(bool)));
}

void ProviderConfigWidget::initRegisterPage()
{
    QString header;
    if (m_provider.name().isEmpty()) {
        header = i18n("Register new account");
    } else {
        header = i18n("Register new account at %1", m_provider.name());
    }
    m_ui.titleWidgetRegister->setText(header);
    m_ui.tabWidget->setTabIcon(registerTabIdx, KIcon("list-add-user"));

    m_ui.infoLabelRP->setFont(KGlobalSettings::smallestReadableFont());

    connect(m_ui.userEditRP, SIGNAL(textChanged(QString)), SLOT(onRegisterDataChanged()));
    connect(m_ui.mailEdit, SIGNAL(textChanged(QString)), SLOT(onRegisterDataChanged()));
    connect(m_ui.firstNameEdit, SIGNAL(textChanged(QString)), SLOT(onRegisterDataChanged()));
    connect(m_ui.lastNameEdit, SIGNAL(textChanged(QString)), SLOT(onRegisterDataChanged()));
    connect(m_ui.passwordEditRP, SIGNAL(textChanged(QString)), SLOT(onRegisterDataChanged()));
    connect(m_ui.passwordRepeatEdit, SIGNAL(textChanged(QString)), SLOT(onRegisterDataChanged()));

    connect(m_ui.registerButton, SIGNAL(clicked()), SLOT(onRegisterClicked()));

    onRegisterDataChanged();
}

void ProviderConfigWidget::onLoginChanged()
{
    m_ui.testLoginButton->setText(i18n("Test login"));
    m_ui.testLoginButton->setEnabled(true);
    emit changed(true);
}

void ProviderConfigWidget::onTestLogin()
{
    m_ui.testLoginButton->setEnabled(false);
    m_ui.testLoginButton->setText(i18n("Testing login..."));

    Attica::PostJob* postJob = m_provider.checkLogin(m_ui.userEditLP->text(), m_ui.passwordEditLP->text());
    connect(postJob, SIGNAL(finished(Attica::BaseJob*)), SLOT(onTestLoginFinished(Attica::BaseJob*)));
    postJob->start();
}

void ProviderConfigWidget::onTestLoginFinished(Attica::BaseJob* job)
{
    Attica::PostJob* postJob = static_cast<Attica::PostJob*>(job);

    if (postJob->metadata().error() == Attica::Metadata::NoError) {
        m_ui.testLoginButton->setText(i18n("Success"));
    }

    if (postJob->metadata().error() == Attica::Metadata::OcsError) {
        m_ui.testLoginButton->setText(i18n("Login failed"));
    }
}

void ProviderConfigWidget::enableToggled(bool enabled)
{
    m_provider.setEnabled(enabled);
}

void ProviderConfigWidget::onInfoLinkActivated()
{
    m_ui.tabWidget->setCurrentIndex(registerTabIdx);
    m_ui.userEditRP->setFocus();
}

void ProviderConfigWidget::onRegisterDataChanged()
{
    QString login = m_ui.userEditRP->text();
    QString mail = m_ui.mailEdit->text();
    QString firstName = m_ui.firstNameEdit->text();
    QString lastName = m_ui.lastNameEdit->text();
    QString password = m_ui.passwordEditRP->text();
    QString passwordRepeat = m_ui.passwordRepeatEdit->text();

    bool isDataValid = (!login.isEmpty() && !mail.isEmpty() && !firstName.isEmpty() &&
                        !lastName.isEmpty() && !password.isEmpty());
    bool isPasswordLengthValid = password.size() > 7;
    bool isPasswordEqual = password == passwordRepeat;

    if (!isDataValid) {
        showRegisterHint("dialog-cancel", i18n("Not all required fields are filled"));
    } else if (!isPasswordLengthValid) {
        showRegisterHint("dialog-cancel", i18n("Password is too short"));
    } else if (!isPasswordEqual) {
        showRegisterHint("dialog-cancel", i18n("Passwords do not match"));
    } else {
        showRegisterHint("dialog-ok-apply", i18n("All required information is provided"));
    }

    m_ui.registerButton->setEnabled(isDataValid && isPasswordLengthValid && isPasswordEqual);

    emit changed(true);
}

void ProviderConfigWidget::showRegisterHint(const QString& iconName, const QString& hint)
{
    m_ui.iconLabelRP->setPixmap(KIcon(iconName).pixmap(16,16));
    m_ui.infoLabelRP->setText(hint);
}

void ProviderConfigWidget::showRegisterError(const Attica::Metadata& metadata)
{
    if (metadata.error() == Attica::Metadata::NetworkError) {
        showRegisterHint("dialog-close", i18n("Failed to register new account."));
    } else {
        /*
# 100 - successful / valid account
# 101 - please specify all mandatory fields
# 102 - please specify a valid password
# 103 - please specify a valid login
# 104 - login already exists
# 105 - email already taken
*/
        // TODO: Looks like more correct place for this stuff is in libattica,
        // for example metadata().statusString() or smth like that.
        // So here will be only showRegisterHint("dialog-close", statusString);
        // no switch.
        QWidget* widgetToHighlight = 0;
        QString hint;
        switch (metadata.statusCode()) {
            case 102:
                hint = i18n("Failed to register new account: invalid password.");
                widgetToHighlight = m_ui.passwordEditRP;
                break;
            case 103:
                hint = i18n("Failed to register new account: invalid username.");
                widgetToHighlight = m_ui.userEditRP;
                break;
            case 104:
                hint = i18n("Failed to register new account: the requested username is already taken.");
                widgetToHighlight = m_ui.userEditRP;
                break;
            case 105:
                hint = i18n("Failed to register new account: the specified email address is already taken.");
                widgetToHighlight = m_ui.mailEdit;
                break;
            case 106:
                hint = i18n("Failed to register new account: the specified email address is invalid.");
                widgetToHighlight = m_ui.mailEdit;
            default:
                hint = i18n("Failed to register new account.");
                break;
        }

        if (!hint.isEmpty())
            showRegisterHint("dialog-close", hint);

        if (widgetToHighlight) {
            QPalette pal = widgetToHighlight->palette();
            KColorScheme::adjustBackground(pal, KColorScheme::NegativeBackground, QPalette::Base);
            widgetToHighlight->setPalette(pal);
            widgetToHighlight->setFocus();
        }
    }
}

void ProviderConfigWidget::clearHighlightedErrors()
{
    QList<QWidget*> widList = allRegisterWidgets();
    foreach (QWidget* wid, widList) {
        QPalette pal = wid->palette();
        KColorScheme::adjustBackground(pal, KColorScheme::NormalBackground, QPalette::Base);
        wid->setPalette(pal);
    }
}

void ProviderConfigWidget::onRegisterClicked()
{
    // here we assume that all data has been checked in onRegisterDataChanged()

    clearHighlightedErrors();

    QString login = m_ui.userEditRP->text();
    QString mail = m_ui.mailEdit->text();
    QString firstName = m_ui.firstNameEdit->text();
    QString lastName = m_ui.lastNameEdit->text();
    QString password = m_ui.passwordEditRP->text();
    //QString passwordRepeat = m_ui.passwordRepeatEdit->text();

    Attica::PostJob* postJob = m_provider.registerAccount(login, password, mail, firstName, lastName);
    connect(postJob, SIGNAL(finished(Attica::BaseJob*)), SLOT(onRegisterAccountFinished(Attica::BaseJob*)));
    postJob->start();
    showRegisterHint("help-about", i18n("Registration is in progress..."));
    m_ui.registerButton->setEnabled(false); // should be disabled while registering
}

void ProviderConfigWidget::onRegisterAccountFinished(Attica::BaseJob* job)
{
    Attica::PostJob* postJob = static_cast<Attica::PostJob*>(job);

    // this will enable "register" button if possible
    onRegisterDataChanged();

    if (postJob->metadata().error() == Attica::Metadata::NoError)
    {
        KMessageBox::information(this, i18n("Registration complete. New account was successfully registered. Please <b>check your Email</b> to <b>activate</b> the account."));

        QString user = m_ui.userEditRP->text();
        QString password = m_ui.passwordEditRP->text();
        m_ui.userEditLP->setText(user);
        m_ui.passwordEditLP->setText(password);

        // clear register fields and switch to login page
        foreach (QWidget* wid, allRegisterWidgets())
        {
            QLineEdit* le = qobject_cast<QLineEdit*>(wid);
            if (le)
                le->clear();
        }
        m_ui.tabWidget->setCurrentIndex(loginTabIdx);
        m_ui.userEditLP->setFocus();
    }
    else
    {
        kDebug() << "register error:" << postJob->metadata().error() << "statusCode:" << postJob->metadata().statusCode();
        showRegisterError( postJob->metadata() );
    }
}

void ProviderConfigWidget::saveData()
{
    m_provider.saveCredentials(m_ui.userEditLP->text(), m_ui.passwordEditLP->text());
}

QList<QWidget*> ProviderConfigWidget::allRegisterWidgets() const
{
    QList<QWidget*> widList;
    widList << m_ui.userEditRP << m_ui.mailEdit << m_ui.firstNameEdit
        << m_ui.lastNameEdit << m_ui.passwordEditRP << m_ui.passwordRepeatEdit;

    return widList;
}

#include "providerconfigwidget.moc"
