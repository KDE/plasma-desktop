/*****************************************************************************
 * Copyright (C) 2011 by Vishesh Yadav <vishesh3y@gmail.com>                 *
 * Copyright (C) 2011 by Peter Penz <peter.penz19@gmail.com>                 *
 *                                                                           *
 * This library is free software; you can redistribute it and/or             *
 * modify it under the terms of the GNU Library General Public               *
 * License version 2 as published by the Free Software Foundation.           *
 *                                                                           *
 * This library is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public License *
 * along with this library; see the file COPYING.LIB.  If not, write to      *
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
 * Boston, MA 02110-1301, USA.                                               *
 *****************************************************************************/

#include "kversioncontrolplugin2.h"
#include <KFileItem>
#include <KFileItemList>

KVersionControlPlugin2::KVersionControlPlugin2(QObject* parent) :
    KVersionControlPlugin(parent)
{
}

KVersionControlPlugin2::~KVersionControlPlugin2()
{
}

KVersionControlPlugin2::VersionState KVersionControlPlugin2::versionState(const KFileItem& item)
{
    Q_UNUSED(item);
    return KVersionControlPlugin::UnversionedVersion;
}

QList<QAction*> KVersionControlPlugin2::contextMenuActions(const KFileItemList& items)
{
    Q_UNUSED(items);
    return QList<QAction*>();
}

QList<QAction*> KVersionControlPlugin2::contextMenuActions(const QString& directory)
{
    Q_UNUSED(directory);
    return QList<QAction*>();
}

#include "kversioncontrolplugin2.moc"
