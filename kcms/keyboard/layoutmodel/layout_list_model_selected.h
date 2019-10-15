#ifndef LAYOUTLISTMODELSELECTED_H
#define LAYOUTLISTMODELSELECTED_H

#include <QAbstractListModel>

#include "layout_list_model_base.h"

class LayoutListModelSelected : public QAbstractListModel, public LayoutListModelBase {
    Q_OBJECT

public:
    struct EnabledLayout
    {
        QString saveName;
        bool isLatinModeEnabled;
        QString latinModeLayout;
    };

    explicit LayoutListModelSelected(QObject* parent = nullptr);

    // Basic functionality:
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    QList<EnabledLayout> enabledLayouts() const;
    void setEnabledLayouts(const QList<EnabledLayout> &enabledLayouts);

    void add(QString const& saveName);
    void remove(int row);

    void setOrder(QVector<int> const& order);

private:
    QList<EnabledLayout> m_enabledLayouts;
};

#endif // LAYOUTLISTMODELSELECTED_H
