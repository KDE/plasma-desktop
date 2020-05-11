/***************************************************************************
 *   Copyright (C) 2020 by Méven Car <meven.car@enioka.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/
#ifndef AUTOSTARTMODEL_H
#define AUTOSTARTMODEL_H

#include <QAbstractListModel>

#include <KService>

enum AutostartEntrySource {
    XdgAutoStart = 0,
    XdgScripts = 1,
    PlasmaShutdown = 2,
    PlasmaStart = 3,
};

struct AutostartEntry
{
    QString name; // Human readable name or local script name
    QString command; // exec or original .sh file
    AutostartEntrySource source;
    bool enabled;
    QString fileName; // the file backing the entry
    bool onlyInPlasma;
};
Q_DECLARE_TYPEINFO(AutostartEntry, Q_MOVABLE_TYPE);

class AutostartModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit AutostartModel(QWidget *parent = nullptr);

    enum Roles {
        Command = Qt::UserRole + 1,
        Enabled,
        Source,
        FileName,
        OnlyInPlasma
    };

    static AutostartEntrySource sourceFromInt(int index);
    static QString XdgAutoStartPath();
    static QStringList listPath();
    static QStringList listPathName();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    bool addEntry(const KService::Ptr &service);

    bool addEntry(const QUrl &path, const bool symlink);

    bool reloadEntry(const QModelIndex &index, const QString &fileName);
    bool removeEntry(const QModelIndex &index);

    void load();

private:
    QVector<AutostartEntry> m_entries;
    QWidget *m_window;
};

#endif // AUTOSTARTMODEL_H
