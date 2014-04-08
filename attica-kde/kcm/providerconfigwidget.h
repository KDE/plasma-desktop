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

#ifndef PROVIDERCONFIGWIDGET_H
#define PROVIDERCONFIGWIDGET_H

#include <QWidget>

#include <attica/provider.h>

#include "ui_providerconfigwidget.h"

namespace Attica
{
    class Metadata;
}

class ProviderConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProviderConfigWidget(QWidget* parent = 0);
    void setProvider(const Attica::Provider& provider);
    void saveData();

Q_SIGNALS:
    void changed(bool hasChanged);

private Q_SLOTS:
    // login page
    void onLoginChanged();
    void onTestLogin();
    void onTestLoginFinished(Attica::BaseJob* job);
    void enableToggled(bool enabled);

    // register page
    void onInfoLinkActivated();
    void onRegisterDataChanged();
    void onRegisterClicked();
    void onRegisterAccountFinished(Attica::BaseJob* job);
    
    
private:
    void initLoginPage();
    void initRegisterPage();
    void showRegisterHint(const QString&, const QString&);
    void showRegisterError(const Attica::Metadata&);
    void clearHighlightedErrors();

    QList<QWidget*> allRegisterWidgets() const;

private:
    Attica::Provider m_provider;
    Ui::ProviderConfigWidget m_ui;
};

#endif
