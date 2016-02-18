/*
 *   Copyright (C) 2016 <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Self
#include "sortedactivitiesmodel.h"

// Qt
#include <QColor>

// KDE
#include <KConfig>
#include <KConfigGroup>
#include <KDirWatch>
#include <KLocalizedString>
#include <KActivities/Consumer>

namespace {

    class BackgroundCache {
    public:
        BackgroundCache()
            : initialized(false)
            , plasmaConfig("plasma-org.kde.plasma.desktop-appletsrc")
        {
            using namespace std::placeholders;

            const auto configFile = QStandardPaths::writableLocation(
                                        QStandardPaths::GenericConfigLocation) +
                                    QLatin1Char('/') + plasmaConfig.name();

            KDirWatch::self()->addFile(configFile);

            QObject::connect(KDirWatch::self(), &KDirWatch::dirty,
                             [this] (const QString &file) { settingsFileChanged(file); });
            QObject::connect(KDirWatch::self(), &KDirWatch::created,
                             [this] (const QString &file) { settingsFileChanged(file); });
        }

        void settingsFileChanged(const QString &file)
        {
            if (!file.endsWith(plasmaConfig.name())) return;

            plasmaConfig.reparseConfiguration();

            if (initialized) {
                reload(false);
            }
        }

        void subscribe(SortedActivitiesModel *model)
        {
            if (!initialized) {
                reload(true);
            }

            models << model;
        }

        void unsubscribe(SortedActivitiesModel *model)
        {
            models.removeAll(model);

            if (models.isEmpty()) {
                initialized = false;
                forActivity.clear();
            }
        }

        QString backgroundFromConfig(const KConfigGroup &config) const
        {
            auto wallpaperPlugin = config.readEntry("wallpaperplugin");
            auto wallpaperConfig = config.group("Wallpaper").group(wallpaperPlugin).group("General");

            if (wallpaperConfig.hasKey("Image")) {
                // Trying for the wallpaper
                auto wallpaper = wallpaperConfig.readEntry("Image", QString());
                if (!wallpaper.isEmpty()) {
                    return wallpaper;
                }
            }
            if (wallpaperConfig.hasKey("Color")) {
                auto backgroundColor = wallpaperConfig.readEntry("Color", QColor(0, 0, 0));
                return backgroundColor.name();
            }

            return QString();
        }

        void reload(bool fullReload)
        {
            QHash<QString, QString> newBackgrounds;

            if (fullReload) {
                forActivity.clear();
            }

            QStringList changedBackgrounds;

            for (const auto &cont: plasmaConfigContainments().groupList()) {

                auto config = plasmaConfigContainments().group(cont);
                auto activityId = config.readEntry("activityId", QString());

                // Ignore if it has no assigned activity
                if (activityId.isEmpty()) continue;

                // Ignore if we have already found the background
                if (newBackgrounds.contains(activityId) &&
                    newBackgrounds[activityId][0] != '#') continue;

                auto newBackground = backgroundFromConfig(config);

                if (forActivity[activityId] != newBackground) {
                    changedBackgrounds << activityId;
                    if (!newBackground.isEmpty()) {
                        newBackgrounds[activityId] = newBackground;
                    }
                }
            }

            initialized = true;

            if (!changedBackgrounds.isEmpty()) {
                forActivity = newBackgrounds;

                for (auto model: models) {
                    model->onBackgroundsUpdated(changedBackgrounds);
                }
            }
        }

        KConfigGroup plasmaConfigContainments() {
            return plasmaConfig.group("Containments");
        }

        QHash<QString, QString> forActivity;
        QList<SortedActivitiesModel*> models;

        bool initialized;
        KConfig plasmaConfig;

    };

    static BackgroundCache &backgrounds()
    {
        // If you convert this to a shared pointer,
        // fix the connections to KDirWatcher
        static BackgroundCache cache;
        return cache;
    }
}

SortedActivitiesModel::SortedActivitiesModel(QVector<KActivities::Info::State> states, QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_sortByLastUsedTime(true)
    , m_activitiesModel(new KActivitiesBackport::ActivitiesModel(states, this))
    , m_activities(new KActivities::Consumer(this))
{
    setSourceModel(m_activitiesModel);

    setDynamicSortFilter(true);
    setSortRole(LastTimeUsed);
    sort(0, Qt::DescendingOrder);

    backgrounds().subscribe(this);
}

SortedActivitiesModel::~SortedActivitiesModel()
{
    backgrounds().unsubscribe(this);
}

bool SortedActivitiesModel::sortByLastUsedTime() const
{
    return m_sortByLastUsedTime;
}

void SortedActivitiesModel::setSortByLastUsedTime(bool sortByLastUsedTime)
{
    if (m_sortByLastUsedTime != sortByLastUsedTime) {
        m_sortByLastUsedTime = sortByLastUsedTime;

        if (m_sortByLastUsedTime) {
            setSortRole(LastTimeUsed);
        } else {
            setSortRole(Qt::DisplayRole);
        }
    }
}

bool SortedActivitiesModel::inhibitUpdates() const
{
    return m_inhibitUpdates;
}

void SortedActivitiesModel::setInhibitUpdates(bool inhibitUpdates)
{
    if (m_inhibitUpdates != inhibitUpdates) {
        m_inhibitUpdates = inhibitUpdates;
        emit inhibitUpdatesChanged(m_inhibitUpdates);

        setDynamicSortFilter(!inhibitUpdates);
    }
}

uint SortedActivitiesModel::lastUsedTime(const QString &activity) const
{
    if (m_activities->currentActivity() == activity) {
        return ~(uint)0;

    } else {
        KConfig config("kactivitymanagerd-switcher");
        KConfigGroup times(&config, "LastUsed");

        return times.readEntry(activity, (uint)0);
    }
}

bool SortedActivitiesModel::lessThan(const QModelIndex &sourceLeft,
                                     const QModelIndex &sourceRight) const
{
    if (m_sortByLastUsedTime) {
        const auto activityLeft  = sourceModel()->data(sourceLeft, KActivitiesBackport::ActivitiesModel::ActivityId);
        const auto activityRight = sourceModel()->data(sourceRight, KActivitiesBackport::ActivitiesModel::ActivityId);

        const auto timeLeft  = lastUsedTime(activityLeft.toString());
        const auto timeRight = lastUsedTime(activityRight.toString());

        return timeLeft < timeRight;

    } else {
        const auto titleLeft  = sourceModel()->data(sourceLeft, KActivitiesBackport::ActivitiesModel::ActivityName);
        const auto titleRight = sourceModel()->data(sourceRight, KActivitiesBackport::ActivitiesModel::ActivityName);

        return titleLeft < titleRight;
    }
}

QHash<int, QByteArray> SortedActivitiesModel::roleNames() const
{
    if (!sourceModel()) return QHash<int, QByteArray>();

    auto roleNames = sourceModel()->roleNames();

    roleNames[LastTimeUsed]       = "lastTimeUsed";
    roleNames[LastTimeUsedString] = "lastTimeUsedString";

    return roleNames;
}

QVariant SortedActivitiesModel::data(const QModelIndex &index, int role) const
{
    if (role == KActivitiesBackport::ActivitiesModel::ActivityBackground) {
        const auto activity =
            QSortFilterProxyModel::data(index, Qt::UserRole).toString();

        return backgrounds().forActivity[activity];

    } else if (role == LastTimeUsed || role == LastTimeUsedString) {
        const auto activity =
            QSortFilterProxyModel::data(index, Qt::UserRole).toString();

        const auto time = lastUsedTime(activity);

        if (role == LastTimeUsed) {
            return QVariant(time);

        } else {
            const auto now = QDateTime::currentDateTime().toTime_t();

            if (time == 0) return i18n("Used some time ago");

            auto diff = now - time;

            // We do not need to be precise
            diff /= 60;
            const auto minutes = diff % 60; diff /= 60;
            const auto hours   = diff % 24; diff /= 24;
            const auto days    = diff % 30; diff /= 30;
            const auto months  = diff % 12; diff /= 12;
            const auto years   = diff;

            return (years > 0)   ? i18n("Used a long time ago")
                 : (months > 0)  ? i18ncp("amount in months",  "Used a month ago",  "Used %1 months ago", months)
                 : (days > 0)    ? i18ncp("amount in days",    "Used a day ago",    "Used %1 days ago",   days)
                 : (hours > 0)   ? i18ncp("amount in hours",   "Used an hour ago",  "Used %1 hours ago",  hours)
                 : (minutes > 0) ? i18ncp("amount in minutes", "Used a minute ago", "Used %1 minutes ago",  minutes)
                 :                 i18n("Used a moment ago");

        }

    } else {
        return QSortFilterProxyModel::data(index, role);
    }
}

QString SortedActivitiesModel::activityIdForRow(int row) const
{
    return data(index(row, 0), KActivitiesBackport::ActivitiesModel::ActivityId).toString();
}

int SortedActivitiesModel::rowForActivityId(const QString &activity) const
{
    int position = -1;

    for (int row = 0; row < rowCount(); ++row) {
        if (activity == activityIdForRow(row)) {
            position = row;
        }
    }

    return position;
}

QString SortedActivitiesModel::relativeActivity(int relative) const
{
    const auto currentActivity = m_activities->currentActivity();

    if (!sourceModel()) return QString();

    const auto currentRowCount = sourceModel()->rowCount();

    int currentActivityRow = 0;

    for (; currentActivityRow < currentRowCount; currentActivityRow++) {
        if (activityIdForRow(currentActivityRow) == currentActivity) break;
    }

    currentActivityRow = (currentActivityRow + relative) % currentRowCount;

    return activityIdForRow(currentActivityRow);
}

void SortedActivitiesModel::onCurrentActivityChanged(const QString &currentActivity)
{
    if (m_previousActivity == currentActivity) return;

    const int previousActivityRow = rowForActivityId(m_previousActivity);
    emit rowChanged(previousActivityRow, { LastTimeUsed, LastTimeUsedString });

    m_previousActivity = currentActivity;

    const int currentActivityRow = rowForActivityId(m_previousActivity);
    emit rowChanged(currentActivityRow, { LastTimeUsed, LastTimeUsedString });
}

void SortedActivitiesModel::onBackgroundsUpdated(const QStringList &activities)
{
    for (const auto &activity: activities) {
        const int row = rowForActivityId(activity);
        emit rowChanged(row, { KActivitiesBackport::ActivitiesModel::ActivityBackground });
    }
}

void SortedActivitiesModel::rowChanged(int row, const QVector<int> &roles)
{
    if (row == -1) return;
    emit dataChanged(index(row, 0), index(row, 0), roles);
}
