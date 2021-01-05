/***************************************************************************
 *   Copyright (C) 2020 Tobias Fella <fella@posteo.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#ifndef KCMCOMPONENTCHOOSER_H
#define KCMCOMPONENTCHOOSER_H

#include <KQuickAddons/ManagedConfigModule>

#include "componentchooser.h"

class KcmComponentChooser : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(ComponentChooser *browsers MEMBER m_browsers CONSTANT)
    Q_PROPERTY(ComponentChooser *emailClients MEMBER m_emailClients CONSTANT)
    Q_PROPERTY(ComponentChooser *terminalEmulators MEMBER m_terminalEmulators CONSTANT)
    Q_PROPERTY(ComponentChooser *fileManagers MEMBER m_fileManagers CONSTANT)

public:
    KcmComponentChooser(QObject *parent, const QVariantList &args);

    void defaults() override;
    void load() override;
    void save() override;
    bool isDefaults() const override;
    bool isSaveNeeded() const override;

private:
    ComponentChooser *m_browsers;
    ComponentChooser *m_emailClients;
    ComponentChooser *m_terminalEmulators;
    ComponentChooser *m_fileManagers;
};

#endif
