#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>
#include <QtCore/QList>
#include <QtCore/QHash>

class DeviceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DeviceModel(QObject *parent = nullptr);
    virtual ~DeviceModel() = default;

    enum DeviceType {
        Attached,
        Detatched
    };

    enum {
        UdiRole = Qt::UserRole,
        TypeRole
    };

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

public slots:
    void forgetDevice(const QString &udi);
    void reload();

private slots:
    void deviceAttached(const QString &udi);
    void deviceRemoved(const QString &udi);

private:
    void addNewDevice(const QString &udi);

    QList<QString> m_attached;
    QList<QString> m_disconnected;
    QHash<QString, bool> m_loginForced;
    QHash<QString, bool> m_attachedForced;
};

#endif
