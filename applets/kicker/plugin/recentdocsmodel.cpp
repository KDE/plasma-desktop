/***************************************************************************
 *   Copyright (C) 2012 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "recentdocsmodel.h"
#include "actionlist.h"

#include <QSet>

#include <KDesktopFile>
#include <KDirWatch>
#include <KFileItem>
#include <KLocalizedString>
#include <KRecentDocument>
#include <KRun>

DocEntry::DocEntry(const QString &name, const QString &icon,
    const QString &url, const QString &desktopPath)
{
    m_name = name;
    m_icon = QIcon::fromTheme(icon, QIcon::fromTheme("unknown"));
    m_url = url;
    m_desktopPath = desktopPath;
}

RecentDocsModel::RecentDocsModel(QObject *parent) : AbstractModel(parent)
{
    KDirWatch *watch = new KDirWatch(this);
    watch->addDir(KRecentDocument::recentDocumentDirectory());

    connect(watch, SIGNAL(created(QString)), SLOT(refresh()));
    connect(watch, SIGNAL(deleted(QString)), SLOT(refresh()));
    connect(watch, SIGNAL(dirty(QString)), SLOT(refresh()));

    refresh();
}

RecentDocsModel::~RecentDocsModel()
{
    qDeleteAll(m_entryList);
}

QVariant RecentDocsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entryList.count()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_entryList.at(index.row())->name();
    } else if (role == Qt::DecorationRole) {
        return m_entryList.at(index.row())->icon();
    } else if (role == Kicker::HasActionListRole) {
        return true;
    } else if (role == Kicker::ActionListRole) {
        KFileItem item(QUrl(m_entryList.at(index.row())->url()));
        QVariantList actionList = Kicker::createActionListForFileItem(item);

        actionList.prepend(Kicker::createSeparatorActionItem());

        const QVariantMap &forgetAllAction = Kicker::createActionItem(i18n("Forget All Documents"), "forgetAll");
        actionList.prepend(forgetAllAction);

        const QVariantMap &forgetAction = Kicker::createActionItem(i18n("Forget Document"), "forget");
        actionList.prepend(forgetAction);

        return actionList;
    }

    return QVariant();
}

int RecentDocsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entryList.count();
}

bool RecentDocsModel::trigger(int row, const QString &actionId, const QVariant &argument)
{
    if (row < 0 || row >= m_entryList.count()) {
        return false;
    }

    QUrl url(m_entryList.at(row)->url());

    if (actionId.isEmpty()) {
        new KRun(url, 0);

        return true;
    } else if (actionId == "forget") {
        forget(row);

        return false;
    } else if (actionId == "forgetAll") {
        forgetAll();

        return true;
    }

    bool close = false;

    KFileItem item(url);

    if (Kicker::handleFileItemAction(item, actionId, argument, &close)) {
        return close;
    }

    return false;
}

void RecentDocsModel::refresh()
{
    beginResetModel();

    qDeleteAll(m_entryList);
    m_entryList.clear();

    QSet<QString> urls;

    foreach (const QString &path, KRecentDocument::recentDocuments()) {
        KDesktopFile file(path);
        QString url = file.readUrl();

        if (urls.contains(url)) {
            continue;
        }

        QString name = file.readName();

        if (name.isEmpty()) {
            name = url;
        }

        if (name.isEmpty()) {
            continue;
        }

        m_entryList << new DocEntry(name, file.readIcon(), url, path);

        urls.insert(url);
    }

    endResetModel();

    emit countChanged();
}

void RecentDocsModel::forget(int row)
{
    if (row >= m_entryList.count()) {
        return;
    }
    if (QFile::remove(m_entryList.at(row)->desktopPath())) {
        beginRemoveRows(QModelIndex(), row, row);

        m_entryList.removeAt(row);

        endRemoveRows();

        emit countChanged();
    }
}

void RecentDocsModel::forgetAll()
{
    beginResetModel();

    for (int row = m_entryList.count() - 1; row >= 0; --row) {
        if (QFile::remove(m_entryList.at(row)->desktopPath())) {
            m_entryList.removeAt(row);
        }
    }

    endResetModel();

    emit countChanged();
}
