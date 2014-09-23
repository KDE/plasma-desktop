/***************************************************************************
 *   Copyright (C) 2013 by Aurélien Gâteau <agateau@kde.org>               *
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

#include <QVariant>

class KFileItem;

namespace Kicker
{

enum {
    IsParentRole = Qt::UserRole + 1,
    HasChildrenRole,
    FavoriteIdRole,
    HasActionListRole,
    ActionListRole,
    UrlRole
};

QVariantMap createActionItem(const QString &label, const QString &actionId, const QVariant &argument = QVariant());

QVariantMap createTitleActionItem(const QString &label);

QVariantMap createSeparatorActionItem();

QVariantList createActionListForFileItem(const KFileItem &fileItem);

bool handleFileItemAction(const KFileItem &fileItem, const QString &actionId, const QVariant &argument, bool *close);

}
