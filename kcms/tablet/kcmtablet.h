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

#include "inputsequence.h"
#include "tabletsmodel.h"

Q_DECLARE_OPAQUE_POINTER(WacomDeviceDatabase *)

class TabletSettings;
class TabletData;

class Tablet : public KQuickManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(TabletsModel *tabletsModel READ tabletsModel CONSTANT)
    Q_PROPERTY(WacomDeviceDatabase *db READ db CONSTANT)

public:
    explicit Tablet(QObject *parent, const KPluginMetaData &metaData);
    ~Tablet() override;

    void load() override;
    void save() override;
    void defaults() override;
    bool isSaveNeeded() const override;
    bool isDefaults() const override;

    TabletsModel *tabletsModel() const;

    Q_SCRIPTABLE void assignPadButtonMapping(const QString &deviceName, uint button, const InputSequence &keySequence);
    Q_SCRIPTABLE void assignPadDialMapping(const QString &deviceName, uint button, const InputSequence &keySequence);
    Q_SCRIPTABLE void assignPadRingMapping(const QString &deviceName, uint button, uint group, const InputSequence &keySequence);
    Q_SCRIPTABLE void assignPadStripMapping(const QString &deviceName, uint button, uint group, const InputSequence &keySequence);
    Q_SCRIPTABLE void assignToolButtonMapping(const QString &deviceName, uint button, const InputSequence &keySequence);
    Q_SCRIPTABLE InputSequence padButtonMapping(const QString &deviceName, uint button) const;
    Q_SCRIPTABLE InputSequence padDialMapping(const QString &deviceName, uint button) const;
    Q_SCRIPTABLE InputSequence padRingMapping(const QString &deviceName, uint button, uint mode) const;
    Q_SCRIPTABLE InputSequence padStripMapping(const QString &deviceName, uint button, uint mode) const;
    Q_SCRIPTABLE InputSequence toolButtonMapping(const QString &deviceName, uint button) const;

    Q_SCRIPTABLE QString toSerializedCurve(const QPointF &controlPoint1, const QPointF &controlPoint2);
    // QML does not support QPair, so let's use a QList as a workaround
    Q_SCRIPTABLE QList<QPointF> fromSerializedCurve(const QString &curve);

    WacomDeviceDatabase *db() const;

Q_SIGNALS:
    void settingsRestored();

private:
    void refreshNeedsSave();

    TabletsModel *m_tabletsModel;
    QHash<QString, QHash<QString, QHash<QPair<uint, uint>, InputSequence>>> m_unsavedMappings;
    WacomDeviceDatabase *m_db = nullptr;
};
