#include "layout_list_models.h"

#include <KConfigGroup>
#include <KProcess>
#include <KSharedConfig>
#include <QDBusArgument>
#include <QDebug>

#include "layout_list_concat_proxy_model.h"
#include "layout_list_filter_disabled_proxy_model.h"
#include "layout_list_filter_duplicates_proxy_model.h"
#include "layout_list_filter_source_proxy_model.h"
#include "layout_list_model_fcitx.h"
#include "layout_list_model_selected.h"
#include "layout_list_model_xkb.h"
#include "layout_list_sort_by_priority_proxy_model.h"
#include "layout_list_sort_filter_proxy_model.h"
#include "layout_list_xkb_expand_proxy_model.h"
#include "layout_list_current_proxy_model.h"
#include "input_sources.h"

LayoutListModels::LayoutListModels(QObject* parent)
    : QObject(parent)
    , m_kdedIface(new QDBusInterface("org.kde.keyboard", "/Layouts", "org.kde.KeyboardLayouts"))
    , m_fcitxIface(new QDBusInterface("org.fcitx.Fcitx", "/inputmethod", "org.freedesktop.DBus.Properties"))
{
    // Setup models
    
    /*
     * Layout organization:
     * 
     * LayoutListModelXkb
     *        |
     * LayoutListXkbExpandProxyModel (flatten tree)    LayoutListModelFcitx      LayoutListModelSelected
     *        |                                            |                          |
     *         \___________________________________________|_________________________/
     *                                                     |
     *                                         LayoutListConcatProxyModel
     *                                                     |
     *                                      LayoutListFilterSourceProxyModel
     *                                                     |
     *                                        LayoutListSortFilterProxyModel // sort by name
     *                                                     |
     *                                     LayoutListFilterDuplicatesProxyModel // remove duplicate
     *                                                     |
     *                                      LayoutListFilterDisabledProxyModel // removed disabled
     *                                                     |
     *                                     LayoutListSortByPriorityProxyModel // sorty by priority
     *                                                     |
     *                                         LayoutListCurrentProxyModel
     */
    auto xkbModel = new LayoutListModelXkb(this);
    auto expanded_xkb = new LayoutListXkbExpandProxyModel(this);
    expanded_xkb->setSourceModel(xkbModel);

    auto fcitxModel = new LayoutListModelFcitx(this);
    m_selected = new LayoutListModelSelected(this);

    auto concat = new LayoutListConcatProxyModel(this);
    concat->addSourceModel(expanded_xkb);
    concat->addSourceModel(fcitxModel);
    concat->addSourceModel(m_selected);

    auto filterSourceModel = new LayoutListFilterSourceProxyModel(this);
    filterSourceModel->setSourceModel(concat);

    auto sortBySaveNameModel = new LayoutListSortFilterProxyModel(this);
    sortBySaveNameModel->setSourceModel(filterSourceModel);
    sortBySaveNameModel->setSortRole(LayoutListModelBase::Roles::SaveNameRole);
    sortBySaveNameModel->sort(0);
    sortBySaveNameModel->setDynamicSortFilter(true);

    auto removeDuplicatesModel = new LayoutListFilterDuplicatesProxyModel(this);
    m_layoutListModel = removeDuplicatesModel;
    removeDuplicatesModel->setSourceModel(sortBySaveNameModel);

    connect(removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::itemAdded,
        m_selected, &LayoutListModelSelected::add);

    connect(removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::itemAdded,
        removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::invalidate);

    connect(m_selected, &LayoutListModelSelected::modelReset,
        removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::updateEnabled);

    connect(m_selected, &LayoutListModelSelected::modelReset,
        removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::missingCountChanged);

    // TODO: make it update without including fcitxdetector here for extensibility
    // TODO: make it work without updating the entire model
    connect(InputSources::self(), &InputSources::currentSourceChanged,
        removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::invalidate);

    connect(InputSources::self(), &InputSources::currentSourceChanged,
        removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::missingCountChanged);

    auto filterDisabledModel = new LayoutListFilterDisabledProxyModel(this);
    filterDisabledModel->setSourceModel(removeDuplicatesModel);

    m_configuredLayoutListModel = new LayoutListSortByPriorityProxyModel(this);
    m_configuredLayoutListModel->setSourceModel(filterDisabledModel);

    connect(m_configuredLayoutListModel, &LayoutListSortByPriorityProxyModel::enabledOrderChanged,
        m_selected, &LayoutListModelSelected::setOrder);

    connect(m_configuredLayoutListModel, &LayoutListSortByPriorityProxyModel::itemRemoved,
        m_selected, &LayoutListModelSelected::remove);

    connect(m_configuredLayoutListModel, &LayoutListSortByPriorityProxyModel::itemRemoved,
        removeDuplicatesModel, &LayoutListFilterDuplicatesProxyModel::invalidate);

    m_currentLayoutListModel = new LayoutListCurrentProxyModel(this);
    m_currentLayoutListModel->setSourceModel(m_configuredLayoutListModel);

    connect(m_kdedIface, SIGNAL(currentLayoutChanged(QString)), this, SLOT(setCurrentLayoutByName(QString)));
    connect(m_fcitxIface, SIGNAL(PropertiesChanged(QString, QVariantMap, QStringList)), this, SLOT(fcitxPropertyChanged(QString, QVariantMap, QStringList)));
}

LayoutListFilterDuplicatesProxyModel *LayoutListModels::entireLayoutListModel() const
{
    return m_layoutListModel;
}

LayoutListSortByPriorityProxyModel *LayoutListModels::configuredLayoutListModel() const
{
    return m_configuredLayoutListModel;
}

LayoutListCurrentProxyModel *LayoutListModels::currentLayoutListModel() const
{
    return m_currentLayoutListModel;
}

int LayoutListModels::currentLayoutIndex()
{
    return m_currentLayoutIndex;
}

void LayoutListModels::setCurrentLayoutIndex(int new_index)
{
    qDebug() << new_index;

    if (m_currentLayoutIndex != new_index) {
        m_currentLayoutIndex = new_index;
        emit currentLayoutIndexChanged();
    }
}

void LayoutListModels::switchLayout(int new_index)
{
    QString name = m_currentLayoutListModel->data(
                m_currentLayoutListModel->index(new_index, 0),
                LayoutListModelBase::Roles::SaveNameRole).toString();
    m_kdedIface->call("setLayout", name);
}

void LayoutListModels::openConfigDialog()
{
    KProcess dialog;
    dialog << "kcmshell5" << "kcm_keyboard";
    dialog.startDetached();
}

QString LayoutListModels::currentIconName()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(
                "org.fcitx.Fcitx",
                "/StatusNotifierItem",
                QLatin1String("org.freedesktop.DBus.Properties"),
                QLatin1String("Get"));
    msg.setArguments(QVariantList() << "org.kde.StatusNotifierItem" << "IconName");
    QDBusMessage reply = QDBusConnection::sessionBus().call(msg);
    QDBusVariant dbusArgs = reply.arguments().first().value<QDBusVariant>();
    return dbusArgs.variant().toString();
}

void LayoutListModels::setCurrentLayoutByName(const QString &saveName)
{
    qDebug() << saveName;
    int count = m_currentLayoutListModel->rowCount();
    for (int i = 0; i < count; ++i) {
        QString name = m_currentLayoutListModel->data(
                    m_currentLayoutListModel->index(i, 0),
                    LayoutListModelBase::Roles::SaveNameRole).toString();
        if (name == saveName) {
            setCurrentLayoutIndex(i);
            return;
        }
    }
    qWarning() << "Cannot set layout" << saveName;
}

void LayoutListModels::fcitxPropertyChanged(QString const&, QVariantMap const&, QStringList const& invalidated)
{
    qDebug() << invalidated;
    if (invalidated.contains("CurrentIM")) {
        emit currentLayoutIndexChanged();
    }
}

void LayoutListModels::loadConfig()
{
    KConfigGroup config(
        KSharedConfig::openConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals),
        QStringLiteral("Layout"));

    QString layoutsString = config.readEntry<QString>("LayoutList", "");
    QList<LayoutListModelSelected::EnabledLayout> list;

    for (QString layout : layoutsString.split(',')) {
        LayoutListModelSelected::EnabledLayout l;
        l.saveName = layout;
        l.isLatinModeEnabled = false;
        l.latinModeLayout = "";

        if (layout.startsWith("fcitx:")) {
            QString origName = layout.split(":")[1];

            l.isLatinModeEnabled = config.readEntry<bool>(QString("Fcitx%1LatinFallBackEnabled").arg(origName), false);
            if (l.isLatinModeEnabled) {
                l.latinModeLayout = config.readEntry<QString>(QString("Fcitx%1LatinFallBack").arg(origName), "us");
            }
        }

        qDebug() << l.saveName << l.isLatinModeEnabled << l.latinModeLayout;

        list << l;
    }

    m_selected->setEnabledLayouts(list);

    QDBusMessage reply = m_kdedIface->call("getCurrentLayout");
    setCurrentLayoutByName(reply.arguments().first().toString());
}

void LayoutListModels::saveConfig()
{
    KConfigGroup config(
        KSharedConfig::openConfig(QStringLiteral("kxkbrc"), KConfig::NoGlobals),
        QStringLiteral("Layout"));

    QStringList layouts;
    for (LayoutListModelSelected::EnabledLayout enabledLayout : m_selected->enabledLayouts()) {
        layouts << enabledLayout.saveName;
    }
    config.writeEntry<QString>("LayoutList", layouts.join(","));
}
