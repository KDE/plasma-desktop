/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmtablet.h"
#include "calibrationtool.h"
#include "inputdevice.h"
#include "logging.h"
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
        appendRow(new QStandardItem(i18nc("@item:inlistbox Stretch and fill to the screen", "Stretch and Fill")));
        appendRow(new QStandardItem(i18nc("@item:inlistbox Keep aspect ratio and fit within the screen", "Keep Aspect Ratio and Fit")));
        appendRow(new QStandardItem(i18nc("@item:inlistbox Map to portion of the screen", "Map to Portion")));

        setItemRoleNames({{Qt::DisplayRole, "display"}});
    }
};

class StylusButtonsModel : public QStandardItemModel
{
    Q_OBJECT

    Q_PROPERTY(WacomDeviceDatabase *db READ db WRITE setDb NOTIFY dbChanged REQUIRED)
    Q_PROPERTY(InputDevice *device READ device WRITE setDevice NOTIFY deviceChanged REQUIRED)

public:
    enum CustomRoles {
        NameRole = Qt::UserRole,
        LabelRole,
        ValueRole,
    };

    StylusButtonsModel()
    {
        setItemRoleNames({
            {NameRole, "name"},
            {LabelRole, "label"},
            {ValueRole, "value"},
        });
        recalculateItems();
    }

    WacomDeviceDatabase *db() const
    {
        return m_db;
    }

    void setDb(WacomDeviceDatabase *db)
    {
        if (db != m_db) {
            m_db = db;
            Q_EMIT dbChanged();
            recalculateItems();
        }
    }

    InputDevice *device() const
    {
        return m_device;
    }

    void setDevice(InputDevice *device)
    {
        if (m_device != device) {
            m_device = device;
            Q_EMIT deviceChanged();
            recalculateItems();
        }
    }

Q_SIGNALS:
    void dbChanged();
    void deviceChanged();

private:
    void recalculateItems()
    {
        if (!m_device || !m_db) {
            return;
        }

        // When there's no stylus detected in the database, fallback to 3.
        int numButtons = 3;
        const QString sysPath = QStringLiteral("/dev/input/%1").arg(m_device->sysName());

        WacomError *error = libwacom_error_new();
        auto device = libwacom_new_from_path(m_db, sysPath.toLatin1().constData(), WFALLBACK_GENERIC, error);
        if (device == nullptr) {
            qCWarning(KCM_TABLET()) << "Failed to find device in libwacom:" << libwacom_error_get_message(error);
        } else {
            int num_styli = 0;
            const int *styli = libwacom_get_supported_styli(device, &num_styli);
            if (num_styli > 0) {
                const auto stylus = libwacom_stylus_get_for_id(m_db, styli[0]);
                if (stylus != nullptr) {
                    numButtons = libwacom_stylus_get_num_buttons(stylus);
                }
            }
        }
        libwacom_error_free(&error);

        // We currently don't support more than 3 stylus buttons, at the moment
        if (numButtons > 3) {
            numButtons = 3;
            qCWarning(KCM_TABLET)
                << "More than 3 stylus buttons detected! This is currently not supported. If you have such a stylus, please file a bug report.";
        }

        clear();

        for (int i = 0; i < numButtons; i++) {
            auto item = new QStandardItem();
            item->setData(i18nc("Stylus pen button", "Pen button %1:", i + 1), LabelRole);
            item->setData(i18nc("@info Meant to be inserted into an existing sentence like 'configuring pen button X'", "pen button %1", i + 1), NameRole);
            switch (i) {
            case 0:
                item->setData(0x14b, ValueRole); // BTN_STYLUS
                break;
            case 1:
                item->setData(0x14c, ValueRole); // BTN_STYLUS2
                break;
            case 2:
                item->setData(0x149, ValueRole); // BTN_STYLUS3
                break;
            default:
                Q_UNREACHABLE();
            }
            appendRow(item);
        }
    }

    WacomDeviceDatabase *m_db = nullptr;
    InputDevice *m_device = nullptr;
};

Tablet::Tablet(QObject *parent, const KPluginMetaData &metaData)
    : KQuickManagedConfigModule(parent, metaData)
{
    m_db = libwacom_database_new();
    if (m_db == nullptr) {
        qCWarning(KCM_TABLET) << "Failed to initialize libwacom database!";
    }

    m_tabletsModel = new TabletsModel(m_db, this);

    qmlRegisterType<OutputsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OutputsModel");
    qmlRegisterType<OrientationsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OrientationsModel");
    qmlRegisterType<StylusButtonsModel>("org.kde.plasma.tablet.kcm", 1, 1, "StylusButtonsModel");
    qmlRegisterType<TabletEvents>("org.kde.plasma.tablet.kcm", 1, 1, "TabletEvents");
    qmlRegisterUncreatableType<InputDevice>("org.kde.plasma.tablet.kcm", 1, 0, "InputDevice", "Access from C++ only");
    qmlRegisterType<CalibrationTool>("org.kde.plasma.tablet.kcm", 1, 1, "CalibrationTool");
    // This looks weird, but the first type here is the value type. The second type is here is just for the enumerations.
    // Yes, this IS what they suggest you do: https://doc.qt.io/qt-6/qtqml-cppintegration-definetypes.html#value-types-with-enumerations
    qmlRegisterType<InputSequence>("org.kde.plasma.tablet.kcm", 1, 1, "inputSequence");
    qmlRegisterUncreatableMetaObject(InputSequence::staticMetaObject, "org.kde.plasma.tablet.kcm", 1, 1, "InputSequence", "Access to enums & flags only");

    connect(m_tabletsModel, &TabletsModel::needsSaveChanged, this, &Tablet::refreshNeedsSave);
    connect(this, &Tablet::settingsRestored, this, &Tablet::refreshNeedsSave);
}

Tablet::~Tablet()
{
    if (m_db) {
        libwacom_database_destroy(m_db);
    }
}

void Tablet::refreshNeedsSave()
{
    setNeedsSave(isSaveNeeded());
}

bool Tablet::isSaveNeeded() const
{
    return !m_unsavedMappings.isEmpty() || m_tabletsModel->isSaveNeeded();
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
    return m_tabletsModel->isDefaults();
}

void Tablet::load()
{
    m_tabletsModel->load();

    m_unsavedMappings.clear();
    Q_EMIT settingsRestored();
}

void Tablet::save()
{
    m_tabletsModel->save();

    auto generalGroup = KSharedConfig::openConfig("kcminputrc")->group("ButtonRebinds");
    for (const auto &device : QStringList{"Tablet", "TabletTool", "TabletDial"}) {
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
    m_tabletsModel->defaults();

    m_unsavedMappings.clear();
    const auto generalGroup = KSharedConfig::openConfig("kcminputrc")->group("ButtonRebinds");
    for (const auto &deviceType : QStringList{"Tablet", "TabletTool", "TabletDial"}) {
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

void Tablet::assignPadDialMapping(const QString &deviceName, uint button, const InputSequence &keySequence)
{
    m_unsavedMappings["TabletDial"][deviceName][button] = keySequence;
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

InputSequence Tablet::padDialMapping(const QString &deviceName, uint button) const
{
    if (deviceName.isEmpty()) {
        return {};
    }

    if (const auto &device = m_unsavedMappings["TabletDial"][deviceName]; device.contains(button)) {
        return device.value(button);
    }

    const auto cfg = KSharedConfig::openConfig("kcminputrc");
    const auto group = cfg->group("ButtonRebinds").group("TabletDial").group(deviceName);
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

QString Tablet::toSerializedCurve(const QPointF &controlPoint1, const QPointF &controlPoint2)
{
    // Turn (0,0) (1,1) to 0.0,0.0;1.0,1.0;
    QString sCurve;
    sCurve += QString::number(controlPoint1.x());
    sCurve += ",";
    sCurve += QString::number(controlPoint1.y());
    sCurve += ";";

    sCurve += QString::number(controlPoint2.x());
    sCurve += ",";
    sCurve += QString::number(controlPoint2.y());
    sCurve += ";";

    return sCurve;
}

QList<QPointF> Tablet::fromSerializedCurve(const QString &curve)
{
    const QStringList data = curve.split(';');

    QList<QPointF> points;
    for (const QString &pair : data) {
        if (pair.indexOf(',') > -1) {
            points.append({pair.section(',', 0, 0).toDouble(), pair.section(',', 1, 1).toDouble()});
        }
    }

    return points;
}

WacomDeviceDatabase *Tablet::db() const
{
    return m_db;
}

TabletsModel *Tablet::tabletsModel() const
{
    return m_tabletsModel;
}

#include "kcmtablet.moc"
#include "moc_kcmtablet.cpp"
