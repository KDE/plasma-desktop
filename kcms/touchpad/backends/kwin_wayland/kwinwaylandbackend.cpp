/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwinwaylandbackend.h"
#include "kwinwaylandtouchpad.h"

#include <algorithm>

#include <KLocalizedString>

#include <QDBusMessage>
#include <QDBusReply>
#include <QStringList>

#include "devicesmodel.h"
#include "logging.h"

KWinWaylandBackend::KWinWaylandBackend(QObject *parent)
    : TouchpadBackend(parent)
    , m_devicesModel(new KWinDevices::DevicesModel(KWinDevices::DevicesModel::Kind::Pointers, {{QStringLiteral("touchpad"), true}}, this))
{
    setMode(TouchpadInputBackendMode::WaylandLibinput);

    connect(m_devicesModel, &QAbstractItemModel::rowsInserted, this, &KWinWaylandBackend::onDevicesInserted);
    connect(m_devicesModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &KWinWaylandBackend::onDevicesAboutToBeRemoved);

    findTouchpads();
}

KWinWaylandBackend::~KWinWaylandBackend()
{
    qDeleteAll(m_devices);
}

void KWinWaylandBackend::findTouchpads()
{
    for (int i = 0; i < m_devicesModel->rowCount(QModelIndex()); ++i) {
        const QString sysName = m_devicesModel->index(i).data(KWinDevices::DevicesModel::SysNameRole).toString();

        KWinWaylandTouchpad *tp = new KWinWaylandTouchpad(sysName);
        if (!tp->load()) {
            qCCritical(KCM_TOUCHPAD) << "Error on creating touchpad object" << sysName;
            m_errorString = i18n("Critical error on reading fundamental device infos for touchpad %1.", sysName);
            return;
        }
        connect(tp, &KWinWaylandTouchpad::needsSaveChanged, this, &KWinWaylandBackend::needsSaveChanged);
        m_devices.append(tp);
        qCInfo(KCM_TOUCHPAD).nospace() << "Touchpad found:" << sysName;
    }
    if (m_devices.isEmpty()) {
        qCInfo(KCM_TOUCHPAD) << "No Devices found.";
    }
}

bool KWinWaylandBackend::save()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](LibinputCommon *t) {
        return t->save();
    });
}

bool KWinWaylandBackend::load()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](LibinputCommon *t) {
        return t->load();
    });
}

bool KWinWaylandBackend::defaults()
{
    return std::all_of(m_devices.constBegin(), m_devices.constEnd(), [](LibinputCommon *t) {
        return t->defaults();
    });
}

bool KWinWaylandBackend::isSaveNeeded() const
{
    return std::any_of(m_devices.constBegin(), m_devices.constEnd(), [](LibinputCommon *t) {
        return t->isSaveNeeded();
    });
}

void KWinWaylandBackend::onDevicesInserted(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; ++i) {
        const QString sysName = m_devicesModel->index(first, 0, parent).data(KWinDevices::DevicesModel::SysNameRole).toString();

        KWinWaylandTouchpad *tp = new KWinWaylandTouchpad(sysName);
        if (!tp->load()) {
            Q_EMIT deviceAdded(false);
            return;
        }
        m_devices.append(tp);
        qCDebug(KCM_TOUCHPAD).nospace() << "Touchpad connected:" << sysName;
        Q_EMIT deviceAdded(true);
        Q_EMIT inputDevicesChanged();
    }
}

void KWinWaylandBackend::onDevicesAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; ++i) {
        const QString sysName = m_devicesModel->index(first, 0, parent).data(KWinDevices::DevicesModel::SysNameRole).toString();

        QList<LibinputCommon *>::const_iterator it = std::find_if(m_devices.constBegin(), m_devices.constEnd(), [&sysName](LibinputCommon *t) {
            return static_cast<KWinWaylandTouchpad *>(t)->sysName() == sysName;
        });
        if (it == m_devices.cend()) {
            return;
        }

        qCDebug(KCM_TOUCHPAD).nospace() << "Touchpad disconnected:" << sysName;

        int index = it - m_devices.cbegin();
        m_devices.removeAt(index);
        Q_EMIT deviceRemoved(index);
        Q_EMIT inputDevicesChanged();
    }
}

#include "moc_kwinwaylandbackend.cpp"
