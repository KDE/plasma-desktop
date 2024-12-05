/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai UWe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DeviceAutomounterKCM.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QItemSelectionModel>
#include <QStandardItem>
#include <QStandardItemModel>

#include <KAboutData>
#include <KConfigGroup>
#include <Solid/DeviceNotifier>
#include <Solid/StorageVolume>

#include <KPluginFactory>

#include "AutomounterSettings.h"
#include "DeviceAutomounterData.h"
#include "DeviceModel.h"
#include "LayoutSettings.h"

K_PLUGIN_FACTORY_WITH_JSON(DeviceAutomounterKCMFactory, "device_automounter_kcm.json", registerPlugin<DeviceAutomounterKCM>();
                           registerPlugin<DeviceAutomounterData>();)

DeviceAutomounterKCM::DeviceAutomounterKCM(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_settings(new AutomounterSettings(this))
    , m_devices(new DeviceModel(m_settings, this))
{
    setupUi(widget());

    addConfig(m_settings, widget());

    deviceView->setModel(m_devices);

    deviceView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    deviceView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(kcfg_AutomountUnknownDevices, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        m_devices->setAutomaticUnknown(state == Qt::Checked);
    });

    connect(deviceView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DeviceAutomounterKCM::updateForgetDeviceButton);
    connect(forgetDevice, &QAbstractButton::clicked, this, &DeviceAutomounterKCM::forgetSelectedDevices);

    connect(m_devices, &QAbstractItemModel::dataChanged, this, &DeviceAutomounterKCM::updateState);

    connect(widget(), &QObject::destroyed, this, &DeviceAutomounterKCM::saveLayout);
}

void DeviceAutomounterKCM::updateState()
{
    kcfg_AutomountUnknownDevices->setEnabled(m_settings->automountOnLogin() || m_settings->automountOnPlugin());

    unmanagedWidgetChangeState(m_unmanagedChanges || m_settings->usrIsSaveNeeded());
    unmanagedWidgetDefaultState(m_settings->isDefaults());
}

void DeviceAutomounterKCM::updateForgetDeviceButton()
{
    const auto selectedIndexes = deviceView->selectionModel()->selectedIndexes();
    const bool isAnyDettached = std::any_of(selectedIndexes.cbegin(), selectedIndexes.cend(), [](const auto &idx) {
        return idx.data(DeviceModel::TypeRole) == DeviceModel::Detached;
    });
    forgetDevice->setEnabled(isAnyDettached);
}

void DeviceAutomounterKCM::forgetSelectedDevices()
{
    QItemSelectionModel *selected = deviceView->selectionModel();
    int offset = 0;
    while (!selected->selectedIndexes().isEmpty() && selected->selectedIndexes().size() > offset) {
        if (selected->selectedIndexes()[offset].data(DeviceModel::TypeRole) == DeviceModel::Attached) {
            offset++;
        } else {
            m_devices->forgetDevice(selected->selectedIndexes()[offset].data(DeviceModel::UdiRole).toString());
        }
    }

    m_unmanagedChanges = true;
    updateState();
}

void DeviceAutomounterKCM::load()
{
    KCModule::load();

    m_devices->reload();
    loadLayout();

    kcfg_AutomountUnknownDevices->setChecked(m_settings->automountUnknownDevices());

    m_unmanagedChanges = false;
    updateState();
}

void DeviceAutomounterKCM::save()
{
    KCModule::save();

    // Housekeeping before saving.
    // 1. Detect if any of the automount options is set to globally enable automounting
    // 2. Clean-up removed setting groups
    bool enabled = m_settings->automountOnLogin() || m_settings->automountOnPlugin();
    QStringList validDevices;

    for (const QModelIndex &idx : {m_devices->index(DeviceModel::RowAttached, 0), m_devices->index(DeviceModel::RowDetached, 0)}) {
        for (int j = 0; j < m_devices->rowCount(idx); ++j) {
            const QString udi = m_devices->index(j, 0, idx).data(DeviceModel::UdiRole).toString();
            validDevices << udi;
            enabled |= m_settings->deviceSettings(udi)->mountOnLogin() | m_settings->deviceSettings(udi)->mountOnAttach();
        }
    }

    m_settings->setAutomountEnabled(enabled);

    const auto knownDevices = m_settings->knownDevices();
    for (const QString &possibleDevice : knownDevices) {
        if (!validDevices.contains(possibleDevice)) {
            m_settings->removeDeviceGroup(possibleDevice);
        }
    }

    m_settings->save();

    // Now tell kded to automatically load the module if loaded
    QDBusConnection dbus = QDBusConnection::sessionBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded6"),
                                                      QStringLiteral("/kded"),
                                                      QStringLiteral("org.kde.kded6"),
                                                      QStringLiteral("setModuleAutoloading"));
    msg.setArguments({QVariant(QStringLiteral("device_automounter")), QVariant(enabled)});
    dbus.call(msg, QDBus::NoBlock);

    // Load or unload right away
    msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded6"),
                                         QStringLiteral("/kded"),
                                         QStringLiteral("org.kde.kded6"),
                                         enabled ? QStringLiteral("loadModule") : QStringLiteral("unloadModule"));
    msg.setArguments({QVariant(QStringLiteral("device_automounter"))});
    dbus.call(msg, QDBus::NoBlock);
}

void DeviceAutomounterKCM::defaults()
{
    KCModule::defaults();

    m_settings->setDefaults();
    m_devices->updateCheckedColumns();
}

void DeviceAutomounterKCM::saveLayout()
{
    QList<int> widths;
    const int nbColumn = m_devices->columnCount();
    widths.reserve(nbColumn);

    for (int i = 0; i < nbColumn; ++i) {
        widths << deviceView->columnWidth(i);
    }

    LayoutSettings::setHeaderWidths(widths);
    LayoutSettings::setAttachedExpanded(deviceView->isExpanded(m_devices->index(DeviceModel::RowAttached, 0)));
    LayoutSettings::setDetachedExpanded(deviceView->isExpanded(m_devices->index(DeviceModel::RowDetached, 0)));
    LayoutSettings::self()->save();
}

void DeviceAutomounterKCM::loadLayout()
{
    LayoutSettings::self()->load();
    // Reset it first, just in case there isn't any layout saved for a particular column.
    int nbColumn = m_devices->columnCount();
    for (int i = 0; i < nbColumn; ++i) {
        deviceView->resizeColumnToContents(i);
    }

    QList<int> widths = LayoutSettings::headerWidths();
    nbColumn = m_devices->columnCount();
    for (int i = 0; i < nbColumn && i < widths.size(); ++i) {
        deviceView->setColumnWidth(i, widths[i]);
    }

    deviceView->setExpanded(m_devices->index(DeviceModel::RowAttached, 0), LayoutSettings::attachedExpanded());
    deviceView->setExpanded(m_devices->index(DeviceModel::RowDetached, 0), LayoutSettings::detachedExpanded());
}

#include "DeviceAutomounterKCM.moc"

#include "moc_DeviceAutomounterKCM.cpp"
