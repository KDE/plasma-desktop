/*
    SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCMCOMPONENTCHOOSER_H
#define KCMCOMPONENTCHOOSER_H

#include <KQuickAddons/ManagedConfigModule>

#include "componentchooser.h"

class ComponentChooserData;

class KcmComponentChooser : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(ComponentChooser *browsers READ browsers CONSTANT)
    Q_PROPERTY(ComponentChooser *emailClients READ emailClients CONSTANT)
    Q_PROPERTY(ComponentChooser *terminalEmulators READ terminalEmulators CONSTANT)
    Q_PROPERTY(ComponentChooser *fileManagers READ fileManagers CONSTANT)

public:
    KcmComponentChooser(QObject *parent, const QVariantList &args);

    ComponentChooser *browsers() const;
    ComponentChooser *emailClients() const;
    ComponentChooser *terminalEmulators() const;
    ComponentChooser *fileManagers() const;

    void defaults() override;
    void load() override;
    void save() override;
    bool isDefaults() const override;
    bool isSaveNeeded() const override;

private:
    ComponentChooserData *m_data;
};

#endif
