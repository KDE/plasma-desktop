/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmtablet.h"
#include "inputdevice.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QGuiApplication>
#include <QScreen>
#include <QStandardItemModel>

K_PLUGIN_FACTORY_WITH_JSON(TabletFactory, "kcm_touchpad2.json", registerPlugin<Tablet>();)

Tablet::Tablet(QObject *parent, const KPluginMetaData &metaData, const QVariantList &list)
    : KQuickManagedConfigModule(parent, metaData, list)
    , m_model(new DevicesModel("touchpad", this))
    // , m_padsModel(new DevicesModel("tabletPad", this))
{
}

Tablet::~Tablet() = default;

bool Tablet::isDefaults() const
{
    return true;
}

void Tablet::load()
{
}

void Tablet::save()
{
}

void Tablet::defaults()
{
}

DevicesModel *Tablet::touchpadModel() const
{
    return m_model;
}



#include "kcmtablet.moc"
