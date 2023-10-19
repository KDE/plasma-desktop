/*
    SPDX-FileCopyrightText: 2008 Fredrik Höglund <fredrik@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <KPluginMetaData>
#include <QAbstractListModel>
#include <QStringList>

class PreviewPluginsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList checkedPlugins READ checkedPlugins WRITE setCheckedPlugins NOTIFY checkedPluginsChanged)

public:
    explicit PreviewPluginsModel(QObject *parent = nullptr);
    ~PreviewPluginsModel() override;

    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        return m_plugins.size();
    }

    QStringList checkedPlugins() const;
    void setCheckedPlugins(const QStringList &list);

Q_SIGNALS:
    void checkedPluginsChanged() const;

private:
    int indexOfPlugin(const QString &pluginId) const;

    QList<KPluginMetaData> m_plugins;
    QList<bool> m_checkedRows;
};
