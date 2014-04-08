/*
    This file is part of KDE.

    Copyright (c) 2009 Eckhart Wörner <ewoerner@kde.org>
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

#ifndef ATTICAMODULE_H
#define ATTICAMODULE_H

#include <KDE/KCModule>

#include <attica/providermanager.h>

#include "ui_providermanagement.h"


namespace KWallet {
    class Wallet;
}

class AtticaModule : public KCModule
{
    Q_OBJECT

public:
    AtticaModule(QWidget* parent, const QVariantList&);
    ~AtticaModule();
    void save();
    void load();
    void defaults();

private Q_SLOTS:
    void providerAdded(const Attica::Provider&);
    void onDefaultProvidersLoaded();
    void addProvider();
    void removeProvider();
    void providerSelected(int);

private:
    void startLoadingDefaultProviders();

private:
    Ui::ProviderManagement m_ui;
    Attica::ProviderManager m_manager;
};


#endif
