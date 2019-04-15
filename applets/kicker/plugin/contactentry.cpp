/***************************************************************************
 *   Copyright (C) 201 by Eike Hein <hein@kde.org>                         *
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

#include "contactentry.h"
#include "actionlist.h"

#include <QBitmap>
#include <QPainter>

#include <KIconLoader>
#include <KLocalizedString>

#include <KPeople/Widgets/Actions>
#include <KPeople/Widgets/PersonDetailsDialog>
#include <KPeople/PersonData>

ContactEntry::ContactEntry(AbstractModel *owner, const QString &id) : AbstractEntry(owner)
, m_personData(nullptr)
{
    if (!id.isEmpty()) {
        m_personData = new KPeople::PersonData(id);

        QObject::connect(m_personData, &KPeople::PersonData::dataChanged,
            [this] { if (m_owner) { m_owner->entryChanged(this); } }
        );
    }
}

bool ContactEntry::isValid() const {
    return m_personData;
}

QIcon ContactEntry::icon() const
{
    if (m_personData) {
        QPixmap photo = m_personData->photo();
        QBitmap mask(photo.size());
        QPainter painter(&mask);
        mask.fill(Qt::white);
        painter.setBrush(Qt::black);
        painter.drawEllipse(0, 0, mask.width(), mask.height());
        photo.setMask(mask);

        photo = photo.scaled(m_owner->iconSize(), m_owner->iconSize(),
            Qt::KeepAspectRatio, Qt::SmoothTransformation);

        KIconLoader::global()->drawOverlays(QStringList() << m_personData->presenceIconName(), photo, KIconLoader::Panel);

        return QIcon(photo);
    }

    return QIcon::fromTheme(QStringLiteral("unknown"));
}

QString ContactEntry::name() const
{
    if (m_personData) {
        return m_personData->name();
    }

    return QString();
}

QString ContactEntry::id() const
{
    if (m_personData) {
        const QString &id = m_personData->personUri();

        if (id.isEmpty()) {
            const QStringList uris = m_personData->contactUris();

            if (!uris.isEmpty()) {
                return uris.at(0);
            }
        } else {
            return id;
        }
    }

    return QString();
}

QUrl ContactEntry::url() const
{
    if (m_personData) {
        return QUrl(m_personData->personUri());
    }

    return QUrl();
}

bool ContactEntry::hasActions() const
{
    return m_personData;
}

QVariantList ContactEntry::actions() const
{
    QVariantList actionList;

    actionList << Kicker::createActionItem(i18n("Show Contact Information..."), QStringLiteral("showContactInfo"));

    return actionList;
}

bool ContactEntry::run(const QString& actionId, const QVariant &argument)
{
    Q_UNUSED(argument)

    if (!m_personData) {
        return false;
    }

    if (actionId == QLatin1String("showContactInfo")) {
        showPersonDetailsDialog(m_personData->personUri());
    }

    return false;
}

void ContactEntry::showPersonDetailsDialog(const QString &id) {
    KPeople::PersonDetailsDialog *view = new KPeople::PersonDetailsDialog(nullptr);
    KPeople::PersonData *data = new KPeople::PersonData(id, view);
    view->setPerson(data);
    view->setAttribute(Qt::WA_DeleteOnClose);
    view->show();
}
