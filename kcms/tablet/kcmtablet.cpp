/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmtablet.h"
#include "devicesmodel.h"
#include "inputdevice.h"
#include "tabletevents.h"

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <QGuiApplication>
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

        addOrientation(i18n("Primary (default)"), Qt::PrimaryOrientation);
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
        auto it = new QStandardItem(i18n("Follow the active screen"));
        it->setData(screens[0]->physicalSize(), Qt::UserRole + 1); // we use the first display to give an idea
        it->setData(screens[0]->size(), Qt::UserRole + 2);
        appendRow(it);

        for (auto screen : screens) {
            auto geo = screen->geometry();
            auto it = new QStandardItem(i18nc("model - (x,y widthxheight)",
                                              "%1 - (%2,%3 %4×%5)",
                                              screen->model(),
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
};

/// This model lists the different ways the tablet will fit onto its output
class OutputsFittingModel : public QStandardItemModel
{
public:
    OutputsFittingModel()
    {
        appendRow(new QStandardItem(i18n("Fit to Output")));
        appendRow(new QStandardItem(i18n("Fit Output in tablet")));
        appendRow(new QStandardItem(i18n("Custom size")));

        setItemRoleNames({{Qt::DisplayRole, "display"}});
    }
};

Tablet::Tablet(QObject *parent, const KPluginMetaData &metaData, const QVariantList &list)
    : ManagedConfigModule(parent, metaData, list)
    , m_toolsModel(new DevicesModel("tabletTool", this))
    , m_padsModel(new DevicesModel("tabletPad", this))
{
    qmlRegisterType<OutputsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OutputsModel");
    qmlRegisterType<OrientationsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OrientationsModel");
    qmlRegisterType<OutputsFittingModel>("org.kde.plasma.tablet.kcm", 1, 1, "OutputsFittingModel");
    qmlRegisterType<TabletEvents>("org.kde.plasma.tablet.kcm", 1, 1, "TabletEvents");
    qmlRegisterAnonymousType<InputDevice>("org.kde.plasma.tablet.kcm", 1);

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
    const auto group = cfg->group("ButtonRebinds").group("Tablet");
    if (group.isValid()) {
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

    const auto cfg = KSharedConfig::openConfig("kcminputrc");
    auto tabletGroup = cfg->group("ButtonRebinds").group("Tablet");
    for (auto it = m_unsavedMappings.cbegin(), itEnd = m_unsavedMappings.cend(); it != itEnd; ++it) {
        auto group = tabletGroup.group(it.key());
        for (auto itDevice = it->cbegin(), itDeviceEnd = it->cend(); itDevice != itDeviceEnd; ++itDevice) {
            const auto key = itDevice->toString(QKeySequence::PortableText);
            const auto button = QString::number(itDevice.key());
            if (key.isEmpty()) {
                group.deleteEntry(button, KConfig::Notify);
            } else {
                group.writeEntry(button, QStringList{"Key", key}, KConfig::Notify);
            }
        }
    }
    tabletGroup.sync();
    m_unsavedMappings.clear();
}

void Tablet::defaults()
{
    m_toolsModel->defaults();
    m_padsModel->defaults();

    const auto tabletGroup = KSharedConfig::openConfig("kcminputrc")->group("ButtonRebinds").group("Tablet");
    const auto tablets = tabletGroup.groupList();
    for (const auto &tablet : tablets) {
        const auto buttons = tabletGroup.group(tablet).keyList();
        for (const auto &button : buttons) {
            m_unsavedMappings[tablet][button.toUInt()] = {};
        }
    }

    for (auto it = m_unsavedMappings.begin(), itEnd = m_unsavedMappings.end(); it != itEnd; ++it) {
        for (auto itDevice = it->begin(), itDeviceEnd = it->end(); itDevice != itDeviceEnd; ++itDevice) {
            *itDevice = {};
        }
    }
    Q_EMIT settingsRestored();
}

void Tablet::assignPadButtonMapping(const QString &deviceName, uint button, const QKeySequence &keySequence)
{
    m_unsavedMappings[deviceName][button] = keySequence;
    Q_EMIT settingsRestored();
}

QKeySequence Tablet::padButtonMapping(const QString &deviceName, uint button) const
{
    if (deviceName.isEmpty()) {
        return {};
    }

    if (const auto &device = m_unsavedMappings[deviceName]; device.contains(button)) {
        return device.value(button);
    }

    const auto cfg = KSharedConfig::openConfig("kcminputrc");
    const auto group = cfg->group("ButtonRebinds").group("Tablet").group(deviceName);
    const auto sequence = group.readEntry(QString::number(button), QStringList());
    if (sequence.size() != 2) {
        return {};
    }
    return QKeySequence(sequence.constLast());
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
