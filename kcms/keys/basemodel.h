/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QAbstractItemModel>
#include <QKeySequence>
#include <QList>
#include <QSet>

class KConfigBase;

// we need to do this to expose the enum to QML
namespace ComponentNS
{
Q_NAMESPACE
enum ComponentType {
    Application,
    Command,
    SystemService,
    CommonAction,
};
Q_ENUM_NS(ComponentType)
};

using namespace ComponentNS;

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
    ComponentType type;
    QString icon;
    QList<Action> actions;
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
        IsDefaultRole,
        IsRemovableRole,
    };
    Q_ENUM(Roles)

    BaseModel(QObject *parent = nullptr);

    Q_INVOKABLE void addShortcut(const QModelIndex &index, const QKeySequence &shortcut);
    Q_INVOKABLE void disableShortcut(const QModelIndex &index, const QKeySequence &shortcut);
    Q_INVOKABLE void changeShortcut(const QModelIndex &index, const QKeySequence &oldShortcut, const QKeySequence &newShortcut);

    virtual void exportToConfig(KConfigBase &config) = 0;
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
    QList<Component> m_components;
};
