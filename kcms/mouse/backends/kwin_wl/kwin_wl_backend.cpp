/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kwin_wl_backend.h"

#include <algorithm>

#include <KLocalizedString>
#include <KSharedConfig>

#include <QKeySequence>
#include <QStringList>

#include "devicesmodel.h"
#include "logging.h"

KWinWaylandBackend::KWinWaylandBackend()
    : InputBackend()
    , m_devicesModel(new KWinDevices::DevicesModel(KWinDevices::DevicesModel::Kind::Pointers, {{QStringLiteral("touchpad"), false}}, this))
{
    connect(m_devicesModel, &QAbstractItemModel::rowsInserted, this, &KWinWaylandBackend::onDevicesInserted);
    connect(m_devicesModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &KWinWaylandBackend::onDevicesAboutToBeRemoved);

    connect(this, &InputBackend::buttonMappingChanged, this, &InputBackend::needsSaveChanged);

    findDevices();
}

KWinWaylandBackend::~KWinWaylandBackend()
{
}

void KWinWaylandBackend::findDevices()
{
    for (int i = 0; i < m_devicesModel->rowCount(QModelIndex()); ++i) {
        const QString sysName = m_devicesModel->index(i).data(KWinDevices::DevicesModel::SysNameRole).toString();

        auto dev = std::make_unique<KWinWaylandDevice>(sysName);
        if (!dev->init()) {
            qCCritical(KCM_MOUSE) << "Error on creating device object" << sysName;
            m_errorString = i18n("Critical error on reading fundamental device infos of %1.", sysName);
            return;
        }
        connect(dev.get(), &KWinWaylandDevice::needsSaveChanged, this, &InputBackend::needsSaveChanged);
        qCDebug(KCM_MOUSE).nospace() << "Device found:" << sysName;
        m_devices.push_back(std::move(dev));
    }
}

bool KWinWaylandBackend::forAllDevices(bool (KWinWaylandDevice::*f)()) const
{
    bool ok = true;
    for (const auto &device : std::as_const(m_devices)) {
        ok &= (device.get()->*f)();
    }
    return ok;
}

KConfigGroup KWinWaylandBackend::mouseButtonRebindsConfigGroup()
{
    return KSharedConfig::openConfig(u"kcminputrc"_s)->group(u"ButtonRebinds"_s).group(u"Mouse"_s);
}

bool KWinWaylandBackend::save()
{
    KConfigGroup buttonGroup = mouseButtonRebindsConfigGroup();

    for (const auto &[buttonName, variant] : std::as_const(m_buttonMapping).asKeyValueRange()) {
        if (const auto keySequence = variant.value<QKeySequence>(); !keySequence.isEmpty()) {
            const auto value = QStringList{u"Key"_s, keySequence.toString(QKeySequence::PortableText)};
            buttonGroup.writeEntry(buttonName, value, KConfig::Notify);
        } else {
            buttonGroup.deleteEntry(buttonName, KConfig::Notify);
        }
    }

    return forAllDevices(&KWinWaylandDevice::save);
}

bool KWinWaylandBackend::load()
{
    m_loadedButtonMapping.clear();
    const KConfigGroup buttonGroup = mouseButtonRebindsConfigGroup();

    for (int i = 1; i <= 24; ++i) {
        const auto buttonName = QLatin1String("ExtraButton%1").arg(QString::number(i));
        const auto entry = buttonGroup.readEntry(buttonName, QStringList());
        if (entry.size() == 2 && entry.first() == QLatin1String("Key")) {
            if (const auto keySequence = QKeySequence::fromString(entry.at(1), QKeySequence::PortableText); !keySequence.isEmpty()) {
                m_loadedButtonMapping.insert(buttonName, keySequence);
            }
        }
    }
    setButtonMapping(m_loadedButtonMapping);

    return forAllDevices(&KWinWaylandDevice::init);
}

bool KWinWaylandBackend::defaults()
{
    return forAllDevices(&KWinWaylandDevice::defaults);
}

bool KWinWaylandBackend::isSaveNeeded() const
{
    return m_buttonMapping != m_loadedButtonMapping || std::ranges::any_of(std::as_const(m_devices), [](const std::unique_ptr<KWinWaylandDevice> &device) {
               return device->isSaveNeeded();
           });
}

QVariantMap KWinWaylandBackend::buttonMapping() const
{
    return m_buttonMapping;
}

void KWinWaylandBackend::setButtonMapping(const QVariantMap &mapping)
{
    if (m_buttonMapping != mapping) {
        m_buttonMapping = mapping;
        Q_EMIT buttonMappingChanged();
    }
}

void KWinWaylandBackend::onDevicesInserted(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; ++i) {
        const QString sysName = m_devicesModel->index(first, 0, parent).data(KWinDevices::DevicesModel::SysNameRole).toString();

        auto dev = std::make_unique<KWinWaylandDevice>(sysName);
        if (!dev->init()) {
            Q_EMIT deviceAdded(false);
            return;
        }

        connect(dev.get(), &KWinWaylandDevice::needsSaveChanged, this, &InputBackend::needsSaveChanged);
        qCDebug(KCM_MOUSE).nospace() << "Device connected:" << sysName;
        m_devices.push_back(std::move(dev));
        Q_EMIT deviceAdded(true);
        Q_EMIT inputDevicesChanged();
    }
}

void KWinWaylandBackend::onDevicesAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    for (int i = first; i <= last; ++i) {
        const QString sysName = m_devicesModel->index(first, 0, parent).data(KWinDevices::DevicesModel::SysNameRole).toString();

        const auto it = std::ranges::find_if(std::as_const(m_devices), [&sysName](const std::unique_ptr<KWinWaylandDevice> &device) {
            return device->sysName() == sysName;
        });
        if (it == m_devices.cend()) {
            continue;
        }
        const int index = std::distance(m_devices.cbegin(), it);

        // delete at the end of the function, after change signals have been fired
        std::unique_ptr<KWinWaylandDevice> dev = std::move(m_devices[index]);
        m_devices.erase(m_devices.cbegin() + index);

        bool deviceNeededSave = dev->isSaveNeeded();
        disconnect(dev.get(), nullptr, this, nullptr);

        qCDebug(KCM_MOUSE).nospace() << "Device disconnected:" << sysName;

        Q_EMIT deviceRemoved(index);
        Q_EMIT inputDevicesChanged();

        if (deviceNeededSave) {
            Q_EMIT needsSaveChanged();
        }
    }
}

QList<InputDevice *> KWinWaylandBackend::inputDevices() const
{
    QList<InputDevice *> devices;
    devices.reserve(m_devices.size());
    for (const auto &device : m_devices) {
        devices.append(device.get());
    }
    return devices;
}

int KWinWaylandBackend::deviceCount() const
{
    return m_devices.size();
}

QString KWinWaylandBackend::errorString() const
{
    return m_errorString;
}

#include "moc_kwin_wl_backend.cpp"
