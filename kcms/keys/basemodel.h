/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QAbstractItemModel>
#include <QKeySequence>
#include <QList>
#include <QSet>
#include <QtQmlIntegration/qqmlintegration.h>

#include "kglobalaccelmodel_export.h"

class Component;
class KConfigBase;

class BaseModel;
class BaseModelPrivate;

// we need to do this to expose the enum to QML
namespace ComponentNS
{
KGLOBALACCELMODEL_EXPORT Q_NAMESPACE enum ComponentType {
    Application,
    Command,
    SystemService,
    CommonAction,
};
KGLOBALACCELMODEL_EXPORT Q_ENUM_NS(ComponentType)
};

using namespace ComponentNS;

class InverseActionReassignmentSuggestion : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("must be created by the shortcut model")

    Q_PROPERTY(QString inverseId MEMBER m_inverseId CONSTANT)
    Q_PROPERTY(QString inverseDisplayName READ inverseDisplayName CONSTANT)
    Q_PROPERTY(QKeySequence shortcutToReplace READ shortcutToReplace CONSTANT)
    Q_PROPERTY(QKeySequence shortcutSuggestion READ shortcutSuggestion CONSTANT)

public:
    struct Data {
        QString inverseDisplayName;
        QPersistentModelIndex inverseIndex;
        QKeySequence shortcutSuggestion;
        QKeySequence shortcutToReplace;
    };
    InverseActionReassignmentSuggestion(const BaseModel *model, const QString &inverseId, const Data &data);

    Q_INVOKABLE QString shortcutNativeText(const QKeySequence &s) const;
    Q_INVOKABLE QModelIndex inverseActionModelIndex() const;

    QString inverseDisplayName() const;
    QKeySequence shortcutSuggestion() const;
    QKeySequence shortcutToReplace() const;

private:
    QString m_inverseId;
    Data m_data;
};

class KGLOBALACCELMODEL_EXPORT BaseModel : public QAbstractItemModel
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
        SupportsMultipleKeysRole,
        IsRemovableRole,
        InverseActionReassignmentSuggestionRole,
    };
    Q_ENUM(Roles)

    BaseModel(QObject *parent = nullptr);
    ~BaseModel();

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
    QList<Component> &components();
    const QList<Component> &components() const;

private:
    std::unique_ptr<BaseModelPrivate> d;
};
