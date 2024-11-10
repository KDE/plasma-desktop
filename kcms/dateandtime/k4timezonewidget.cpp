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

#include <KCountry>
#include <KCountryFlagEmojiIconEngine>
#include <KLocalizedString>

#include <QDebug>
#include <QFile>
#include <QPixmap>
#include <QTimeZone>

#include <unicode/localebuilder.h>
#include <unicode/tznames.h>
#include <unicode/unistr.h>

class Q_DECL_HIDDEN K4TimeZoneWidget::Private
{
public:
    Private()
        : itemsCheckable(false)
        , singleSelection(true)
    {
    }

    enum Columns {
        CityColumn = 0,
        RegionColumn,
        CommentColumn,
    };

    enum Roles {
        ZoneRole = Qt::UserRole + 0xF3A3CB1,
    };

    bool itemsCheckable;
    bool singleSelection;
};

static bool localeLessThan(const QString &a, const QString &b)
{
    return QString::localeAwareCompare(a, b) < 0;
}

K4TimeZoneWidget::K4TimeZoneWidget(QWidget *parent)
    : QTreeWidget(parent)
    , d(new K4TimeZoneWidget::Private)
{
    // If the user did not provide a timezone database, we'll use the system default.
    setRootIsDecorated(false);
    setHeaderLabels(QStringList() << i18nc("Define an area in the time zone, like a town area", "Area") << i18nc("Time zone", "Region") << i18n("Comment"));

    const auto locale = icu::Locale(QLocale::system().name().toLatin1());
    UErrorCode error = U_ZERO_ERROR;
    std::unique_ptr<icu::TimeZoneNames> tzNames(icu::TimeZoneNames::createInstance(locale, error));
    if (!U_SUCCESS(error)) {
        qWarning() << "failed to create timezone names" << u_errorName(error);
        return;
    };
    icu::UnicodeString result;

    const auto timezones = QTimeZone::availableTimeZoneIds();

    struct TimeZoneInfo {
        QTimeZone zone;
        QString region;
        QString city;
    };

    std::vector<TimeZoneInfo> zones;
    zones.reserve(timezones.size());
    for (const auto &id : timezones) {
        const int separator = id.lastIndexOf('/');
        // Make up the localized key that will be used for sorting.
        // Example: Asia/Tokyo -> key = "i18n(Tokyo)|i18n(Asia)|Asia/Tokyo"
        // The zone name is appended to ensure unicity even with equal translations (#174918)
        // since there is no  API continent names, we use the translation catalog of the digitalclock applet which contain these
        const char *domain = "plasma_applet_org.kde.plasma.digitalclock.mo";
        const QString continent = separator > 0 ? i18nd(domain, id.left(separator)) : QString();
        const icu::UnicodeString &exemplarCity = tzNames->getExemplarLocationName(icu::UnicodeString::fromUTF8(icu::StringPiece(id.data(), id.size())), result);
        zones.push_back(
            {QTimeZone(id), continent, exemplarCity.isBogus() ? QString::fromLatin1(id) : QString::fromUtf16(exemplarCity.getBuffer(), exemplarCity.length())});
    }
    std::sort(zones.begin(), zones.end(), [](const TimeZoneInfo &left, const TimeZoneInfo &right) {
        if (auto result = QString::localeAwareCompare(left.city, right.city); result != 0) {
            return result < 0;
        }
        if (auto result = QString::localeAwareCompare(left.region, right.region); result != 0) {
            return result < 0;
        }
        return QString::localeAwareCompare(left.zone.id(), right.zone.id()) < 0;
    });

    for (const auto &zoneInfo : zones) {
        const QTimeZone &zone = zoneInfo.zone;
        const QString tzName = zone.id();
        QString comment = zone.comment();

        if (!comment.isEmpty()) {
            comment = i18n(comment.toUtf8());
        }

        QTreeWidgetItem *listItem = new QTreeWidgetItem(this);
        listItem->setText(Private::CityColumn, zoneInfo.city);
        QString countryName = KCountry::fromQLocale(zone.territory()).name();
        if (countryName.isEmpty()) {
            countryName = QLocale::territoryToCode(zone.territory());
        }
        QString regionLabel = zoneInfo.region;
        if (!regionLabel.isEmpty() && !countryName.isEmpty()) {
            regionLabel += '/';
        }
        regionLabel += countryName;

        listItem->setText(Private::RegionColumn, regionLabel);
        listItem->setText(Private::CommentColumn, comment);
        listItem->setData(Private::CityColumn, Private::ZoneRole, tzName); // store complete path in custom role

        listItem->setIcon(Private::RegionColumn, QIcon(new KCountryFlagEmojiIconEngine(QLocale::territoryToCode(zone.territory()))));
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

#include "moc_k4timezonewidget.cpp"
