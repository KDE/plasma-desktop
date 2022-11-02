/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KQuickAddons/ManagedConfigModule>

#include <KSharedConfig>
#include <QKeySequence>

#include "devicesmodel.h"

class TabletSettings;
class TabletData;

class Tablet : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(DevicesModel *toolsModel READ toolsModel CONSTANT)
    Q_PROPERTY(DevicesModel *padsModel READ padsModel CONSTANT)

public:
    explicit Tablet(QObject *parent, const KPluginMetaData &metaData, const QVariantList &list);
    ~Tablet() override;

    void load() override;
    void save() override;
    void defaults() override;
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    DevicesModel *toolsModel() const;
    DevicesModel *padsModel() const;

    Q_SCRIPTABLE void assignPadButtonMapping(const QString &deviceName, uint button, const QKeySequence &keySequence);
    Q_SCRIPTABLE void assignToolButtonMapping(const QString &deviceName, uint button, const QKeySequence &keySequence);
    Q_SCRIPTABLE QKeySequence padButtonMapping(const QString &deviceName, uint button) const;
    Q_SCRIPTABLE QKeySequence toolButtonMapping(const QString &deviceName, uint button) const;

Q_SIGNALS:
    void settingsRestored();

private:
    void refreshNeedsSave();

    DevicesModel *const m_toolsModel;
    DevicesModel *const m_padsModel;
    QHash<QString, QHash<QString, QHash<uint, QKeySequence>>> m_unsavedMappings;
};
