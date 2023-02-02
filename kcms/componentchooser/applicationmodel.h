/*
    SPDX-FileCopyrightText: 2022 Méven Car <meven@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef APPLICATION_MODEL_H
#define APPLICATION_MODEL_H

#include <KService>
#include <QAbstractItemModel>

#include <optional>

class ApplicationModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ApplicationModel(QObject *parent = nullptr);

    enum Roles {
        Icon = Qt::UserRole + 1,
        StorageId = Qt::UserRole + 2,
        IsDefault = Qt::UserRole + 3,
        Selected = Qt::UserRole + 4,
        ExecLine = Qt::UserRole + 5,
    };
    Q_ENUMS(Roles)

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant data(const int &row, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    QModelIndex findByStorageId(const QString &storageId) const;

    QHash<int, QByteArray> roleNames() const override;

    void load(const QString &mimeType, const QString &applicationCategory, const QString &defaultApplication, const KService::Ptr preferredService);
    Q_INVOKABLE int currentIndex() const;

    std::optional<int> defaultIndex() const;

    int addApplicationBeforeLast(const KServicePtr service);
    void addApplication(const QString &name, const QString &iconName, const QString &storageId, bool isSelected, const QString &execLine);

private:
    bool isValid(const QModelIndex &index) const;

    QList<QVariantMap> m_applications;
    int m_defaultIndex = -1;
};

#endif // APPLICATION_MODEL_H
