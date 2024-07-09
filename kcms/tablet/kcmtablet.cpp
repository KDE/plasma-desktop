/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmtablet.h"
#include "calibrationtool.h"
#include "inputdevice.h"
#include "tabletevents.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QGuiApplication>
#include <QMatrix4x4>
#include <QScreen>
#include <QStandardItemModel>

K_PLUGIN_FACTORY_WITH_JSON(TabletFactory, "kcm_tablet.json", registerPlugin<Tablet>();)

class OrientationsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    OrientationsModel()
    {
        auto addOrientation = [this](const QString &display, Qt::ScreenOrientation o) {
            auto item = new QStandardItem(display);
            item->setData(o, Qt::UserRole);
            appendRow(item);
        };

        addOrientation(i18n("Default"), Qt::PrimaryOrientation);
        addOrientation(i18n("Portrait"), Qt::PortraitOrientation);
        addOrientation(i18n("Landscape"), Qt::LandscapeOrientation);
        addOrientation(i18n("Inverted Portrait"), Qt::InvertedPortraitOrientation);
        addOrientation(i18n("Inverted Landscape"), Qt::InvertedLandscapeOrientation);

        setItemRoleNames({
            {Qt::DisplayRole, "display"},
            {Qt::UserRole, "value"},
        });
    }

    Q_SCRIPTABLE int orientationAt(int row) const
    {
        return item(row)->data(Qt::UserRole).toInt();
    }
    Q_SCRIPTABLE int rowForOrientation(int orientation)
    {
        for (int i = 0, c = rowCount(); i < c; ++i) {
            if (item(i)->data(Qt::UserRole) == orientation) {
                return i;
            }
        }

        // If not found, let's just return the PrimaryOrientation
        return 0;
    }
};

/// This is for the Fitting tablet to screen, "" means to follow the Active screen, otherwise we pass the name
class OutputsModel : public QStandardItemModel
{
    Q_OBJECT
public:
    OutputsModel()
    {
        setItemRoleNames({
            {Qt::DisplayRole, "display"},
            {Qt::UserRole, "name"},
            {Qt::UserRole + 1, "physicalSize"},
            {Qt::UserRole + 2, "size"},
        });

        reset();

        connect(qGuiApp, &QGuiApplication::screenAdded, this, &OutputsModel::reset);
        connect(qGuiApp, &QGuiApplication::screenRemoved, this, &OutputsModel::reset);
    }

    void reset()
    {
        clear();

        auto screens = qGuiApp->screens();
        auto it = new QStandardItem(i18n("Follow the Current Screen"));
        it->setData(screens[0]->physicalSize(), Qt::UserRole + 1); // we use the first display to give an idea
        it->setData(screens[0]->size(), Qt::UserRole + 2);
        appendRow(it);

        it = new QStandardItem(i18n("All Screens"));
        it->setData(screens[0]->virtualSize(), Qt::UserRole + 1);
        it->setData(screens[0]->virtualSize(), Qt::UserRole + 2);
        appendRow(it);

        for (auto screen : screens) {
            auto geo = screen->geometry();
            auto name = screen->model().isEmpty() ? screen->name() : screen->model();
            auto it = new QStandardItem(i18nc("model - (x,y widthxheight)",
                                              "%1 - (%2,%3 %4×%5)",
                                              name,
                                              QString::number(geo.x()),
                                              QString::number(geo.y()),
                                              QString::number(geo.width()),
                                              QString::number(geo.height())));
            it->setData(screen->name(), Qt::UserRole);
            it->setData(screen->physicalSize(), Qt::UserRole + 1);
            it->setData(screen->size(), Qt::UserRole + 2);
            appendRow(it);
        }

        setItemRoleNames({
            {Qt::DisplayRole, "display"},
            {Qt::UserRole, "name"},
            {Qt::UserRole + 1, "physicalSize"},
            {Qt::UserRole + 2, "size"},
        });
    }

    Q_SCRIPTABLE QString outputNameAt(int row) const
    {
        return item(row)->data(Qt::UserRole).toString();
    }

    Q_SCRIPTABLE int rowForDevice(InputDevice *device)
    {
        if (!device) {
            return 0;
        } else if (device->isMapToWorkspace()) {
            return 1;
        } else {
            return rowForOutputName(device->outputName());
        }
    }

    Q_SCRIPTABLE int rowForOutputName(const QString &outputName)
    {
        for (int i = 0, c = rowCount(); i < c; ++i) {
            if (item(i)->data(Qt::UserRole) == outputName) {
                return i;
            }
        }

        // If not found, let's just return the PrimaryOrientation
        return 0;
    }

    Q_SCRIPTABLE bool isMapToWorkspaceAt(int row)
    {
        return row == 1;
    }
};

/// This model lists the different ways the tablet will fit onto its output
class OutputsFittingModel : public QStandardItemModel
{
public:
    OutputsFittingModel()
    {
        appendRow(new QStandardItem(i18n("Fit to Screen")));
        appendRow(new QStandardItem(i18n("Keep Aspect Ratio and Fit Within Screen")));
        appendRow(new QStandardItem(i18n("Map to Portion of Screen")));

        setItemRoleNames({{Qt::DisplayRole, "display"}});
    }
};

// QMatrix4x4 over DBus is needed for writing calibration matrices
QDBusArgument &operator<<(QDBusArgument &argument, const QMatrix4x4 &matrix)
{
    argument.beginArray(qMetaTypeId<double>());
    for (quint8 row = 0; row < 4; ++row) {
        for (quint8 col = 0; col < 4; ++col) {
            argument << matrix(row, col);
        }
    }
    argument.endArray();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QMatrix4x4 &matrix)
{
    argument.beginArray();
    for (quint8 row = 0; row < 4; ++row) {
        for (quint8 col = 0; col < 4; ++col) {
            double val;
            argument >> val;
            matrix(row, col) = val;
        }
    }
    argument.endArray();
    return argument;
}

Tablet::Tablet(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
    , m_toolsModel(new DevicesModel("tabletTool", this))
    , m_padsModel(new DevicesModel("tabletPad", this))
{
    qDBusRegisterMetaType<QMatrix4x4>();

    qmlRegisterType<OutputsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OutputsModel");
    qmlRegisterType<OrientationsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OrientationsModel");
    qmlRegisterType<OutputsFittingModel>("org.kde.plasma.tablet.kcm", 1, 1, "OutputsFittingModel");
    qmlRegisterType<TabletEvents>("org.kde.plasma.tablet.kcm", 1, 1, "TabletEvents");
    qmlRegisterAnonymousType<InputDevice>("org.kde.plasma.tablet.kcm", 1);
    qmlRegisterType<CalibrationTool>("org.kde.plasma.tablet.kcm", 1, 1, "CalibrationTool");
    // This looks weird, but the first type here is the value type. The second type is here is just for the enumerations.
    // Yes, this IS what they suggest you do: https://doc.qt.io/qt-6/qtqml-cppintegration-definetypes.html#value-types-with-enumerations
    qmlRegisterType<InputSequence>("org.kde.plasma.tablet.kcm", 1, 1, "inputSequence");
    qmlRegisterUncreatableMetaObject(InputSequence::staticMetaObject, "org.kde.plasma.tablet.kcm", 1, 1, "InputSequence", "Access to enums & flags only");

    connect(m_toolsModel, &DevicesModel::needsSaveChanged, this, &Tablet::refreshNeedsSave);
    connect(m_padsModel, &DevicesModel::needsSaveChanged, this, &Tablet::refreshNeedsSave);
    connect(this, &Tablet::settingsRestored, this, &Tablet::refreshNeedsSave);
}

Tablet::~Tablet() = default;

void Tablet::refreshNeedsSave()
{
    setNeedsSave(isSaveNeeded());
}

bool Tablet::isSaveNeeded() const
{
    return !m_unsavedMappings.isEmpty() || m_toolsModel->isSaveNeeded() || m_padsModel->isSaveNeeded();
}

bool Tablet::isDefaults() const
{
    if (!m_unsavedMappings.isEmpty())
        return false;

    const auto cfg = KSharedConfig::openConfig("kcminputrc");
    if (cfg->group("ButtonRebinds").group("Tablet").isValid()) {
        return false;
    }
    if (cfg->group("ButtonRebinds").group("TabletTool").isValid()) {
        return false;
    }
    return m_toolsModel->isDefaults() && m_padsModel->isDefaults();
}

void Tablet::load()
{
    m_toolsModel->load();
    m_padsModel->load();

    m_unsavedMappings.clear();
    Q_EMIT settingsRestored();
}

void Tablet::save()
{
    m_toolsModel->save();
    m_padsModel->save();

    auto generalGroup = KSharedConfig::openConfig("kcminputrc")->group("ButtonRebinds");
    for (const auto &device : QStringList{"Tablet", "TabletTool"}) {
        for (auto it = m_unsavedMappings[device].cbegin(), itEnd = m_unsavedMappings[device].cend(); it != itEnd; ++it) {
            auto group = generalGroup.group(device).group(it.key());
            for (auto itDevice = it->cbegin(), itDeviceEnd = it->cend(); itDevice != itDeviceEnd; ++itDevice) {
                const auto key = itDevice->toConfigFormat();
                const auto button = QString::number(itDevice.key());
                if (key.isEmpty()) {
                    group.deleteEntry(button, KConfig::Notify);
                } else {
                    group.writeEntry(button, key, KConfig::Notify);
                }
            }
        }
    }
    generalGroup.sync();
    m_unsavedMappings.clear();
}

void Tablet::defaults()
{
    m_toolsModel->defaults();
    m_padsModel->defaults();

    m_unsavedMappings.clear();
    const auto generalGroup = KSharedConfig::openConfig("kcminputrc")->group("ButtonRebinds");
    for (const auto &deviceType : QStringList{"Tablet", "TabletTool"}) {
        auto tabletGroup = generalGroup.group(deviceType);
        const auto tablets = tabletGroup.groupList();
        for (const auto &deviceName : tablets) {
            const auto buttons = tabletGroup.group(deviceName).keyList();
            for (const auto &button : buttons) {
                m_unsavedMappings[deviceType][deviceName][button.toUInt()] = {};
            }
        }
    }
    Q_EMIT settingsRestored();
}

void Tablet::assignPadButtonMapping(const QString &deviceName, uint button, const InputSequence &keySequence)
{
    m_unsavedMappings["Tablet"][deviceName][button] = keySequence;
    Q_EMIT settingsRestored();
}

void Tablet::assignToolButtonMapping(const QString &deviceName, uint button, const InputSequence &keySequence)
{
    m_unsavedMappings["TabletTool"][deviceName][button] = keySequence;
    Q_EMIT settingsRestored();
}

InputSequence Tablet::padButtonMapping(const QString &deviceName, uint button) const
{
    if (deviceName.isEmpty()) {
        return {};
    }

    if (const auto &device = m_unsavedMappings["Tablet"][deviceName]; device.contains(button)) {
        return device.value(button);
    }

    const auto cfg = KSharedConfig::openConfig("kcminputrc");
    const auto group = cfg->group("ButtonRebinds").group("Tablet").group(deviceName);
    const auto sequence = group.readEntry(QString::number(button), QStringList());
    return InputSequence(sequence);
}

InputSequence Tablet::toolButtonMapping(const QString &deviceName, uint button) const
{
    if (deviceName.isEmpty()) {
        return {};
    }

    if (const auto &device = m_unsavedMappings["TabletTool"][deviceName]; device.contains(button)) {
        return device.value(button);
    }

    const auto cfg = KSharedConfig::openConfig("kcminputrc");
    const auto group = cfg->group("ButtonRebinds").group("TabletTool").group(deviceName);
    const auto sequence = group.readEntry(QString::number(button), QStringList());
    return InputSequence(sequence);
}

DevicesModel *Tablet::toolsModel() const
{
    return m_toolsModel;
}

DevicesModel *Tablet::padsModel() const
{
    return m_padsModel;
}

#include "kcmtablet.moc"
