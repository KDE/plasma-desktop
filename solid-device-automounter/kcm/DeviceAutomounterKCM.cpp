/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai UWe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DeviceAutomounterKCM.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QItemSelectionModel>
#include <QStandardItem>
#include <QStandardItemModel>
#include <kconfigwidgets_version.h>

#include <KAboutData>
#include <KConfigGroup>
#include <Solid/DeviceNotifier>
#include <Solid/StorageVolume>

#include <KPluginFactory>

#include "AutomounterSettings.h"
#include "DeviceAutomounterData.h"
#include "DeviceModel.h"
#include "LayoutSettings.h"

K_PLUGIN_FACTORY(DeviceAutomounterKCMFactory, registerPlugin<DeviceAutomounterKCM>(); registerPlugin<DeviceAutomounterData>();)

DeviceAutomounterKCM::DeviceAutomounterKCM(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args) // DeviceAutomounterKCMFactory::componentData(), parent)
    , m_settings(new AutomounterSettings(this))
    , m_devices(new DeviceModel(m_settings, this))
{
    KAboutData *about = new KAboutData(QStringLiteral("kcm_device_automounter"),
                                       i18n("Device Automounter"),
                                       QStringLiteral("2.0"),
                                       QString(),
                                       KAboutLicense::GPL_V2,
                                       i18n("(c) 2009 Trever Fischer, (c) 2015 Kai Uwe Broulik"));
    about->addAuthor(i18n("Trever Fischer"), i18n("Original Author"));
    about->addAuthor(i18n("Kai Uwe Broulik"), i18n("Plasma 5 Port"), QStringLiteral("kde@privat.broulik.de"));

    setAboutData(about);
    setupUi(this);

    addConfig(m_settings, this);

    deviceView->setModel(m_devices);

    deviceView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    deviceView->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    connect(kcfg_AutomountOnLogin, &QCheckBox::stateChanged, this, [this](int state) {
        m_devices->setAutomaticMountOnLogin(state == Qt::Checked);
    });
    connect(kcfg_AutomountOnPlugin, &QCheckBox::stateChanged, this, [this](int state) {
        m_devices->setAutomaticMountOnPlugin(state == Qt::Checked);
    });

    connect(deviceView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DeviceAutomounterKCM::updateForgetDeviceButton);

    connect(forgetDevice, &QAbstractButton::clicked, this, &DeviceAutomounterKCM::forgetSelectedDevices);

    forgetDevice->setEnabled(false);
}

DeviceAutomounterKCM::~DeviceAutomounterKCM()
{
    saveLayout();
}

void DeviceAutomounterKCM::updateForgetDeviceButton()
{
    const auto selectedIndex = deviceView->selectionModel()->selectedIndexes();
    for (const QModelIndex &idx : selectedIndex) {
        if (idx.data(DeviceModel::TypeRole) == DeviceModel::Detached) {
            forgetDevice->setEnabled(true);
            return;
        }
    }
    forgetDevice->setEnabled(false);
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

    markAsChanged();
}

void DeviceAutomounterKCM::load()
{
    KCModule::load();

    kcfg_AutomountUnknownDevices->setEnabled(m_settings->automountEnabled());
    kcfg_AutomountOnLogin->setEnabled(m_settings->automountEnabled());
    kcfg_AutomountOnPlugin->setEnabled(m_settings->automountEnabled());

    m_devices->reload();
    loadLayout();
}

void DeviceAutomounterKCM::save()
{
    KCModule::save();
    saveLayout();

    const bool enabled = kcfg_AutomountEnabled->isChecked();

    QStringList validDevices;
    for (int i = 0; i < m_devices->rowCount(); ++i) {
        const QModelIndex &idx = m_devices->index(i, 0);

        for (int j = 0; j < m_devices->rowCount(idx); ++j) {
            QModelIndex dev = m_devices->index(j, 1, idx);
            const QString device = dev.data(DeviceModel::UdiRole).toString();
            validDevices << device;

            if (dev.data(Qt::CheckStateRole).toInt() == Qt::Checked) {
                m_settings->deviceSettings(device).writeEntry("ForceLoginAutomount", true);
            } else {
                m_settings->deviceSettings(device).writeEntry("ForceLoginAutomount", false);
            }

            dev = dev.sibling(j, 2);

            if (dev.data(Qt::CheckStateRole).toInt() == Qt::Checked) {
                m_settings->deviceSettings(device).writeEntry("ForceAttachAutomount", true);
            } else {
                m_settings->deviceSettings(device).writeEntry("ForceAttachAutomount", false);
            }
        }
    }

    const auto knownDevices = m_settings->knownDevices();
    for (const QString &possibleDevice : knownDevices) {
        if (!validDevices.contains(possibleDevice)) {
            m_settings->deviceSettings(possibleDevice).deleteGroup();
        }
    }

    m_settings->save();

    // Now tell kded to automatically load the module if loaded
    QDBusConnection dbus = QDBusConnection::sessionBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                                      QStringLiteral("/kded"),
                                                      QStringLiteral("org.kde.kded5"),
                                                      QStringLiteral("setModuleAutoloading"));
    msg.setArguments({QVariant(QStringLiteral("device_automounter")), QVariant(enabled)});
    dbus.call(msg, QDBus::NoBlock);

    // Load or unload right away
    msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                         QStringLiteral("/kded"),
                                         QStringLiteral("org.kde.kded5"),
                                         enabled ? QStringLiteral("loadModule") : QStringLiteral("unloadModule"));
    msg.setArguments({QVariant(QStringLiteral("device_automounter"))});
    dbus.call(msg, QDBus::NoBlock);
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
    // Check DeviceModel.cpp, thats where the magic row numbers come from.
    LayoutSettings::setAttachedExpanded(deviceView->isExpanded(m_devices->index(0, 0)));
    LayoutSettings::setDetachedExpanded(deviceView->isExpanded(m_devices->index(1, 0)));
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

    deviceView->setExpanded(m_devices->index(0, 0), LayoutSettings::attachedExpanded());
    deviceView->setExpanded(m_devices->index(1, 0), LayoutSettings::detachedExpanded());
}

#include "DeviceAutomounterKCM.moc"
