/*
 * Copyright 2020 David Redondo <kde@david-redondo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QAbstractItemModel>
#include <QKeySequence>
#include <QSet>
#include <QVector>

class KConfigBase;

struct Action {
    QString id;
    QString displayName;
    QSet<QKeySequence> activeShortcuts;
    QSet<QKeySequence> defaultShortcuts;
    QSet<QKeySequence> initialShortcuts;
};

struct Component {
    QString id;
    QString displayName;
    QString type;
    QString icon;
    QVector<Action> actions;
    bool checked;
    bool pendingDeletion;
};

class BaseModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        SectionRole = Qt::UserRole,
        ComponentRole,
        ActionRole,
        ActiveShortcutsRole,
        DefaultShortcutsRole,
        CustomShortcutsRole,
        CheckedRole,
        PendingDeletionRole,
        SupportsMultipleKeysRole
    };
    Q_ENUM(Roles)

    BaseModel(QObject *parent = nullptr);

    Q_INVOKABLE void addShortcut(const QModelIndex &index, const QKeySequence &shortcut);
    Q_INVOKABLE void disableShortcut(const QModelIndex &index, const QKeySequence &shortcut);
    Q_INVOKABLE void changeShortcut(const QModelIndex &index, const QKeySequence &oldShortcut, const QKeySequence &newShortcut);

    virtual void exportToConfig(const KConfigBase &config) = 0;
    virtual void importConfig(const KConfigBase &config) = 0;

    virtual void load() = 0;
    virtual void save() = 0;
    void defaults();
    bool needsSave() const;
    bool isDefault() const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

protected:
    QVector<Component> m_components;
};

#endif
