/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                         *
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

#include "fileentry.h"
#include "actionlist.h"

#include <KFileItem>
#include <KRun>

FileEntry::FileEntry(AbstractModel *owner, const QUrl &url) : AbstractEntry(owner)
, m_fileItem(nullptr)
{
    if (url.isValid()) {
        m_fileItem = new KFileItem(url);
        m_fileItem->determineMimeType();
    }
}

FileEntry::~FileEntry()
{
    delete m_fileItem;
}

bool FileEntry::isValid() const
{
    return m_fileItem && (m_fileItem->isFile() || m_fileItem->isDir());
}

QIcon FileEntry::icon() const
{
    if (m_fileItem) {
        return QIcon::fromTheme(m_fileItem->iconName(), QIcon::fromTheme(QStringLiteral("unknown")));
    }

    return QIcon::fromTheme(QStringLiteral("unknown"));
}

QString FileEntry::name() const
{
    if (m_fileItem) {
        return m_fileItem->text();
    }

    return QString();
}

QString FileEntry::description() const
{
    if (m_fileItem) {
        return m_fileItem->url().toString(QUrl::PreferLocalFile);
    }

    return QString();
}

QString FileEntry::id() const
{
    if (m_fileItem) {
        return m_fileItem->url().toString();
    }

    return QString();
}

QUrl FileEntry::url() const
{
    if (m_fileItem) {
        return m_fileItem->url();
    }

    return QUrl();
}

bool FileEntry::hasActions() const
{
    return m_fileItem && m_fileItem->isFile();
}

QVariantList FileEntry::actions() const
{
    if (m_fileItem) {
        return Kicker::createActionListForFileItem(*m_fileItem);
    }

    return QVariantList();
}

bool FileEntry::run(const QString& actionId, const QVariant &argument)
{
    if (!m_fileItem) {
        return false;
    }

    if (actionId.isEmpty()) {
        new KRun(m_fileItem->url(), nullptr);

        return true;
    } else {
        bool close = false;

        if (Kicker::handleFileItemAction(*m_fileItem, actionId, argument, &close)) {
            return close;
        }
    }

    return false;
}
