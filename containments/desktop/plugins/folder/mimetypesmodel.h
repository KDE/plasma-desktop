/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MIMETYPESMODEL_H
#define MIMETYPESMODEL_H

#include <QAbstractListModel>
#include <QMimeType>

class QStringList;

class MimeTypesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList checkedTypes READ checkedTypes WRITE setCheckedTypes NOTIFY checkedTypesChanged)

public:
    explicit MimeTypesModel(QObject *parent = nullptr);
    ~MimeTypesModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return m_mimeTypesList.size();
    }

    QStringList checkedTypes() const;
    void setCheckedTypes(const QStringList &list);

Q_SIGNALS:
    void checkedTypesChanged() const;

private:
    int indexOfType(const QString &name) const;

    QList<QMimeType> m_mimeTypesList;
    QVector<bool> m_checkedRows;
};

#endif
