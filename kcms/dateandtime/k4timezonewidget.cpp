/*
    Copyright (C) 2005, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2009, David Faure <faure@kde.org>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "k4timezonewidget.h"

#include <QDebug>
#include <QFile>
#include <QPixmap>

#include <klocale.h>
#include <klocalizedstring.h>
#include <ksystemtimezone.h>
#include <ktimezone.h>

class Q_DECL_HIDDEN K4TimeZoneWidget::Private
{
public:
    Private()
        : itemsCheckable(false)
        , singleSelection(true)
    {
    }

    enum Columns { CityColumn = 0, RegionColumn, CommentColumn };

    enum Roles { ZoneRole = Qt::UserRole + 0xF3A3CB1 };

    bool itemsCheckable;
    bool singleSelection;
};

static bool localeLessThan(const QString &a, const QString &b)
{
    return QString::localeAwareCompare(a, b) < 0;
}

K4TimeZoneWidget::K4TimeZoneWidget(QWidget *parent, KTimeZones *db)
    : QTreeWidget(parent)
    , d(new K4TimeZoneWidget::Private)
{
    // If the user did not provide a timezone database, we'll use the system default.
    setRootIsDecorated(false);
    setHeaderLabels(QStringList() << i18nc("Define an area in the time zone, like a town area", "Area") << i18nc("Time zone", "Region") << i18n("Comment"));

    // Collect zones by localized city names, so that they can be sorted properly.
    QStringList cities;
    QHash<QString, KTimeZone> zonesByCity;

    if (!db) {
        db = KSystemTimeZones::timeZones();

        // add UTC to the defaults default
        KTimeZone utc = KTimeZone::utc();
        cities.append(utc.name());
        zonesByCity.insert(utc.name(), utc);
    }

    const KTimeZones::ZoneMap zones = db->zones();
    for (KTimeZones::ZoneMap::ConstIterator it = zones.begin(); it != zones.end(); ++it) {
        const KTimeZone zone = it.value();
        const QString continentCity = displayName(zone);
        const int separator = continentCity.lastIndexOf('/');
        // Make up the localized key that will be used for sorting.
        // Example: i18n(Asia/Tokyo) -> key = "i18n(Tokyo)|i18n(Asia)|Asia/Tokyo"
        // The zone name is appended to ensure unicity even with equal translations (#174918)
        const QString key = continentCity.mid(separator + 1) + '|' + continentCity.left(separator) + '|' + zone.name();
        cities.append(key);
        zonesByCity.insert(key, zone);
    }
    std::sort(cities.begin(), cities.end(), localeLessThan);

    foreach (const QString &key, cities) {
        const KTimeZone zone = zonesByCity.value(key);
        const QString tzName = zone.name();
        QString comment = zone.comment();

        if (!comment.isEmpty()) {
            comment = i18n(comment.toUtf8());
        }

        // Convert:
        //
        //  "Europe/London", "GB" -> "London", "Europe/GB".
        //  "UTC",           ""   -> "UTC",    "".
        QStringList continentCity = displayName(zone).split('/');

        QTreeWidgetItem *listItem = new QTreeWidgetItem(this);
        listItem->setText(Private::CityColumn, continentCity[continentCity.count() - 1]);
        QString countryName = KLocale::global()->countryCodeToName(zone.countryCode());
        if (countryName.isEmpty()) {
            continentCity[continentCity.count() - 1] = zone.countryCode();
        } else {
            continentCity[continentCity.count() - 1] = countryName;
        }

        listItem->setText(Private::RegionColumn, continentCity.join(QChar('/')));
        listItem->setText(Private::CommentColumn, comment);
        listItem->setData(Private::CityColumn, Private::ZoneRole, tzName); // store complete path in custom role

        // Locate the flag from share/kf5/locale/countries/%1/flag.png
        QString flag =
            QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString("kf5/locale/countries/%1/flag.png").arg(zone.countryCode().toLower()));
        if (QFile::exists(flag)) {
            listItem->setIcon(Private::RegionColumn, QPixmap(flag));
        }
    }
}

K4TimeZoneWidget::~K4TimeZoneWidget()
{
    delete d;
}

void K4TimeZoneWidget::setItemsCheckable(bool enable)
{
    d->itemsCheckable = enable;
    const int count = topLevelItemCount();
    for (int row = 0; row < count; ++row) {
        QTreeWidgetItem *listItem = topLevelItem(row);
        listItem->setCheckState(Private::CityColumn, Qt::Unchecked);
    }
    QTreeWidget::setSelectionMode(QAbstractItemView::NoSelection);
}

bool K4TimeZoneWidget::itemsCheckable() const
{
    return d->itemsCheckable;
}

QString K4TimeZoneWidget::displayName(const KTimeZone &zone)
{
    return i18n(zone.name().toUtf8()).replace('_', ' ');
}

QStringList K4TimeZoneWidget::selection() const
{
    QStringList selection;

    // Loop through all entries.
    // Do not use selectedItems() because it skips hidden items, making it
    // impossible to use a KTreeWidgetSearchLine.
    // There is no QTreeWidgetItemConstIterator, hence the const_cast :/
    QTreeWidgetItemIterator it(const_cast<K4TimeZoneWidget *>(this), d->itemsCheckable ? QTreeWidgetItemIterator::Checked : QTreeWidgetItemIterator::Selected);
    for (; *it; ++it) {
        selection.append((*it)->data(Private::CityColumn, Private::ZoneRole).toString());
    }

    return selection;
}

void K4TimeZoneWidget::setSelected(const QString &zone, bool selected)
{
    bool found = false;

    // The code was using findItems( zone, Qt::MatchExactly, Private::ZoneColumn )
    // previously, but the underlying model only has 3 columns, the "hidden" column
    // wasn't available in there.

    if (!d->itemsCheckable) {
        // Runtime compatibility for < 4.3 apps, which don't call the setMultiSelection reimplementation.
        d->singleSelection = (QTreeWidget::selectionMode() == QAbstractItemView::SingleSelection);
    }

    // Loop through all entries.
    const int rowCount = model()->rowCount(QModelIndex());
    for (int row = 0; row < rowCount; ++row) {
        const QModelIndex index = model()->index(row, Private::CityColumn);
        const QString tzName = index.data(Private::ZoneRole).toString();
        if (tzName == zone) {
            if (d->singleSelection && selected) {
                clearSelection();
            }

            if (d->itemsCheckable) {
                QTreeWidgetItem *listItem = itemFromIndex(index);
                listItem->setCheckState(Private::CityColumn, selected ? Qt::Checked : Qt::Unchecked);
            } else {
                selectionModel()->select(index,
                                         selected ? (QItemSelectionModel::Select | QItemSelectionModel::Rows)
                                                  : (QItemSelectionModel::Deselect | QItemSelectionModel::Rows));
            }

            // Ensure the selected item is visible as appropriate.
            scrollTo(index);

            found = true;

            if (d->singleSelection && selected) {
                break;
            }
        }
    }

    if (!found) {
        qDebug() << "No such zone: " << zone;
    }
}

void K4TimeZoneWidget::clearSelection()
{
    if (d->itemsCheckable) {
        // Un-select all items
        const int rowCount = model()->rowCount(QModelIndex());
        for (int row = 0; row < rowCount; ++row) {
            const QModelIndex index = model()->index(row, 0);
            QTreeWidgetItem *listItem = itemFromIndex(index);
            listItem->setCheckState(Private::CityColumn, Qt::Unchecked);
        }
    } else {
        QTreeWidget::clearSelection();
    }
}

void K4TimeZoneWidget::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    d->singleSelection = (mode == QAbstractItemView::SingleSelection);
    if (!d->itemsCheckable) {
        QTreeWidget::setSelectionMode(mode);
    }
}

QAbstractItemView::SelectionMode K4TimeZoneWidget::selectionMode() const
{
    if (d->itemsCheckable) {
        return d->singleSelection ? QTreeWidget::SingleSelection : QTreeWidget::MultiSelection;
    } else {
        return QTreeWidget::selectionMode();
    }
}
