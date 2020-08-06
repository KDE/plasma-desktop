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

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QAbstractItemModel>
#include <QKeySequence>
#include <QDBusPendingCallWatcher>
#include <QList>
#include <QSet>
#include <QVector>


class QDBusError;
class QDBusObjectPath;

class KConfigBase;
class KGlobalAccelInterface;
class KGlobalShortcutInfo;

class FilteredShortcutsModel;

struct Shortcut {
    QString uniqueName;
    QString friendlyName;
    QSet<QKeySequence> activeShortcuts;
    QSet<QKeySequence> defaultShortcuts;
    QSet<QKeySequence> initialShortcuts;
};

struct Component {
    QString uniqueName;
    QString friendlyName;
    QString type;
    QString icon;
    QVector<Shortcut> shortcuts;
    bool checked;
    bool pendingDeletion;
};

class ShortcutsModel : public QAbstractItemModel
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
        PendingDeletionRole
    };
    Q_ENUM(Roles)

    ShortcutsModel(KGlobalAccelInterface *interface, QObject *parent = nullptr);

    Q_INVOKABLE void toggleDefaultShortcut(const QModelIndex &index, const QKeySequence &shortcut, bool enabled);
    Q_INVOKABLE void addShortcut(const QModelIndex &index, const QKeySequence &shortcut);
    Q_INVOKABLE void disableShortcut(const QModelIndex &index, const QKeySequence &shortcut);
    Q_INVOKABLE void changeShortcut(const QModelIndex &index, const QKeySequence &oldShortcut, const QKeySequence &newShortcut);

    void setShortcuts(const KConfigBase &config);
    void addApplication(const QString &desktopFileName, const QString &displayName);

    void load();
    void defaults();
    void save();
    bool needsSave() const;
    bool isDefault() const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

Q_SIGNALS:
    void errorOccured(const QString&);

private:
    Component loadComponent(const QList<KGlobalShortcutInfo> &info);
    void removeComponent(const Component &component);
    void genericErrorOccured(const QString &description, const QDBusError &error);

    KGlobalAccelInterface *m_globalAccelInterface;
    QVector<Component> m_components;
};

#endif // SHORTCUTSMODEL_H
