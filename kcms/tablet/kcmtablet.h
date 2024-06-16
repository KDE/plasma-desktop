/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <KQuickManagedConfigModule>

#include <KSharedConfig>

extern "C" {
#include <libwacom/libwacom.h>
}

#include "devicesmodel.h"
#include "inputsequence.h"

Q_DECLARE_OPAQUE_POINTER(WacomDeviceDatabase *)

class TabletSettings;
class TabletData;

class Tablet : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(DevicesModel *toolsModel READ toolsModel CONSTANT)
    Q_PROPERTY(DevicesModel *padsModel READ padsModel CONSTANT)
    Q_PROPERTY(WacomDeviceDatabase *db READ db CONSTANT)

public:
    explicit Tablet(QObject *parent, const KPluginMetaData &metaData);
    ~Tablet() override;

    void load() override;
    void save() override;
    void defaults() override;
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    DevicesModel *toolsModel() const;
    DevicesModel *padsModel() const;

    Q_SCRIPTABLE void assignPadButtonMapping(const QString &deviceName, uint button, const InputSequence &keySequence);
    Q_SCRIPTABLE void assignToolButtonMapping(const QString &deviceName, uint button, const InputSequence &keySequence);
    Q_SCRIPTABLE InputSequence padButtonMapping(const QString &deviceName, uint button) const;
    Q_SCRIPTABLE InputSequence toolButtonMapping(const QString &deviceName, uint button) const;

    Q_SCRIPTABLE QString toSerializedCurve(const QPointF &controlPoint1, const QPointF &controlPoint2);
    // QML does not support QPair, so let's use a QList as a workaround
    Q_SCRIPTABLE QList<QPointF> fromSerializedCurve(const QString &curve);

    WacomDeviceDatabase *db() const;

Q_SIGNALS:
    void settingsRestored();

private:
    void refreshNeedsSave();

    DevicesModel *const m_toolsModel;
    DevicesModel *const m_padsModel;
    QHash<QString, QHash<QString, QHash<uint, InputSequence>>> m_unsavedMappings;
    WacomDeviceDatabase *m_db = nullptr;
};
