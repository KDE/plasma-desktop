/*
    SPDX-FileCopyrightText: 2009 Ben Cooksley <ben@eclipse.endoftheinternet.org>
    SPDX-FileCopyrightText: 2007 Will Stephenson <wstephenson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ActionModel.h"
#include "ActionItem.h"

#include <KDesktopFileActions>

#include <QDirIterator>
#include <QIcon>
#include <QStandardPaths>

class ActionModel::Private
{
public:
    Private()
    {
    }

    QList<ActionItem *> actions;
};

static bool sortAction(ActionItem *left, ActionItem *right)
{
    return left->name().localeAwareCompare(right->name()) < 0;
}

ActionModel::ActionModel(QObject *parent)
    : QAbstractTableModel(parent)
    , d(new Private())
{
}

ActionModel::~ActionModel()
{
    qDeleteAll(d->actions);
    d->actions.clear();
    delete d;
}

int ActionModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

int ActionModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->actions.count();
    }
    return 0;
}

QVariant ActionModel::data(const QModelIndex &index, int role) const
{
    QVariant theData;
    if (!index.isValid()) {
        return QVariant();
    }

    ActionItem *mi = d->actions.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0) {
            theData.setValue(mi->name());
        } else if (index.column() == 1) {
            theData.setValue(mi->involvedTypes());
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            theData = QIcon::fromTheme(mi->icon());
        }
        break;
    case Qt::UserRole:
        theData.setValue(mi);
        break;
    default:
        break;
    }
    return theData;
}

void ActionModel::buildActionList()
{
    beginResetModel();
    qDeleteAll(d->actions);
    d->actions.clear();
    // Prepare to search for possible actions -> we only want solid types
    const QStringList actionDirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("solid/actions"), QStandardPaths::LocateDirectory);
    // Get service objects for those actions and add them to the display
    for (const QString &actionDir : actionDirs) {
        QDirIterator it(actionDir, QStringList() << QStringLiteral("*.desktop"));
        while (it.hasNext()) {
            it.next();
            const QString desktop = it.filePath();
            // Get contained services list
            const QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(desktop, true);
            for (const KServiceAction &deviceAction : services) {
                ActionItem *actionItem = new ActionItem(desktop, deviceAction.name(), this); // Create an action
                d->actions.append(actionItem);
            }
        }
    }

    std::sort(d->actions.begin(), d->actions.end(), sortAction);
    endResetModel();
}

QList<ActionItem *> ActionModel::actionList() const
{
    return d->actions;
}
