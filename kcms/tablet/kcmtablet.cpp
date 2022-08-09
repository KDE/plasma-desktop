/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcmtablet.h"
#include "devicesmodel.h"
#include "inputdevice.h"
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
        auto screens = qGuiApp->screens();
        auto it = new QStandardItem(i18n("Follow the active screen"));
        it->setData(screens[0]->physicalSize(), Qt::UserRole + 1); // we use the first display to give an idea
        it->setData(screens[0]->size(), Qt::UserRole + 2);
        appendRow(it);

        for (auto screen : screens) {
            auto geo = screen->geometry();
            auto it =
                new QStandardItem(i18nc("model - (x,y widthxheight)", "%1 - (%2,%3 %4×%5)", screen->model(), geo.x(), geo.y(), geo.width(), geo.height()));
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
    , m_devicesModel(new DevicesModel(this))
{
    qmlRegisterType<OutputsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OutputsModel");
    qmlRegisterType<OrientationsModel>("org.kde.plasma.tablet.kcm", 1, 0, "OrientationsModel");
    qmlRegisterType<OutputsFittingModel>("org.kde.plasma.tablet.kcm", 1, 1, "OutputsFittingModel");
    qmlRegisterAnonymousType<InputDevice>("org.kde.plasma.tablet.kcm", 1);

    connect(m_devicesModel, &DevicesModel::needsSaveChanged, this, &Tablet::refreshNeedsSave);
}

Tablet::~Tablet() = default;

void Tablet::refreshNeedsSave()
{
    setNeedsSave(isSaveNeeded());
}

bool Tablet::isSaveNeeded() const
{
    return m_devicesModel->isSaveNeeded();
}

bool Tablet::isDefaults() const
{
    return m_devicesModel->isDefaults();
}

void Tablet::load()
{
    m_devicesModel->load();
}
void Tablet::save()
{
    m_devicesModel->save();
}
void Tablet::defaults()
{
    m_devicesModel->defaults();
}

DevicesModel *Tablet::devicesModel() const
{
    return m_devicesModel;
}

#include "kcmtablet.moc"
