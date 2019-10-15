#ifndef LAYOUT_LIST_MODELS_H
#define LAYOUT_LIST_MODELS_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusArgument>

#include "layout_list_model_selected.h"
#include "layout_list_filter_duplicates_proxy_model.h"
#include "layout_list_sort_by_priority_proxy_model.h"
#include "layout_list_current_proxy_model.h"

class LayoutListModels : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int currentLayoutIndex READ currentLayoutIndex WRITE setCurrentLayoutIndex NOTIFY currentLayoutIndexChanged)

    LayoutListModelSelected* m_selected;
    LayoutListFilterDuplicatesProxyModel* m_layoutListModel;
    LayoutListSortByPriorityProxyModel* m_configuredLayoutListModel;
    LayoutListCurrentProxyModel* m_currentLayoutListModel;
    QDBusInterface* m_kdedIface, *m_fcitxIface;
    int m_currentLayoutIndex;

public:
    LayoutListModels(QObject *parent);

    LayoutListFilterDuplicatesProxyModel *entireLayoutListModel() const;
    LayoutListSortByPriorityProxyModel *configuredLayoutListModel() const;
    LayoutListCurrentProxyModel *currentLayoutListModel() const;

    int currentLayoutIndex();
    void setCurrentLayoutIndex(int new_index);
    Q_INVOKABLE void switchLayout(int new_index);
    Q_INVOKABLE void openConfigDialog();
    QString currentIconName();

    void loadConfig();
    void saveConfig();

Q_SIGNALS:
    void currentLayoutIndexChanged();

public Q_SLOTS:
    void setCurrentLayoutByName(QString const& saveName);
    void fcitxPropertyChanged(QString const& ifaceName, QVariantMap const& map, QStringList const& invalidated);
};

#endif // LAYOUT_LIST_MODELS_H
