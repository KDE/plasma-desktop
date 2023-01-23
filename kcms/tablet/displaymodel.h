

#pragma once

#include <QAbstractListModel>
#include <memory>

class DDCDisplay;
class DisplayModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles { Name = Qt::UserRole + 1, Manufacturer, DDCRef };
    DisplayModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<std::unique_ptr<DDCDisplay>> m_displays;
};
