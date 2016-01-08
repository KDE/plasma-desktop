/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
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

#include "ResourcesDatabaseSchema.h"

#include <QStandardPaths>
#include <QVariant>
#include <QCoreApplication>

namespace Common {
namespace ResourcesDatabaseSchema {

const QString name = QStringLiteral("Resources");

QString version()
{
    return QStringLiteral("2015.02.09");
}

QStringList schema()
{
    // If only we could use initializer lists here ...

    return QStringList()

        << // Schema informations table, used for versioning
           QStringLiteral("CREATE TABLE IF NOT EXISTS SchemaInfo ("
               "key text PRIMARY KEY, value text"
           ")")

        << QStringLiteral("INSERT OR IGNORE INTO schemaInfo VALUES ('version', '%1')").arg(version())
        << QStringLiteral("UPDATE schemaInfo SET value = '%1' WHERE key = 'version'").arg(version())


        << // The ResourceEvent table saves the Opened/Closed event pairs for
           // a resource. The Accessed event is mapped to those.
           // Focussing events are not stored in order not to get a
           // huge database file and to lessen writes to the disk.
           QStringLiteral("CREATE TABLE IF NOT EXISTS ResourceEvent ("
               "usedActivity TEXT, "
               "initiatingAgent TEXT, "
               "targettedResource TEXT, "
               "start INTEGER, "
               "end INTEGER "
           ")")

        << // The ResourceScoreCache table stores the calcualted scores
           // for resources based on the recorded events.
           QStringLiteral("CREATE TABLE IF NOT EXISTS ResourceScoreCache ("
               "usedActivity TEXT, "
               "initiatingAgent TEXT, "
               "targettedResource TEXT, "
               "scoreType INTEGER, "
               "cachedScore FLOAT, "
               "firstUpdate INTEGER, "
               "lastUpdate INTEGER, "
               "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
           ")")


        << // @since 2014.05.05
           // The ResourceLink table stores the information, formerly kept by
           // Nepomuk, of which resources are linked to which activities.
           // The additional features compared to the old days are
           // the ability to limit the link to specific applications, and
           // to create global links.
           QStringLiteral("CREATE TABLE IF NOT EXISTS ResourceLink ("
               "usedActivity TEXT, "
               "initiatingAgent TEXT, "
               "targettedResource TEXT, "
               "PRIMARY KEY(usedActivity, initiatingAgent, targettedResource)"
           ")")

        << // @since 2015.01.18
           // The ResourceInfo table stores the collected information about a
           // resource that is not agent nor activity related like the
           // title and the mime type.
           // If these are automatically retrieved (works for files), the
           // flag is set to true. This is done for the agents to be able to
           // override these.
           QStringLiteral("CREATE TABLE IF NOT EXISTS ResourceInfo ("
               "targettedResource TEXT, "
               "title TEXT, "
               "mimetype TEXT, "
               "autoTitle INTEGER, "
               "autoMimetype INTEGER, "
               "PRIMARY KEY(targettedResource)"
           ")")

       ;
}

// TODO: This will require some refactoring after we introduce more databases
QString defaultPath()
{
    return QStandardPaths::writableLocation(
               QStandardPaths::GenericDataLocation)
           + QStringLiteral("/kactivitymanagerd/resources/database");
}

const char *overrideFlagProperty = "org.kde.KActivities.ResourcesDatabase.overrideDatabase";
const char *overrideFileProperty = "org.kde.KActivities.ResourcesDatabase.overrideDatabaseFile";

QString path()
{
    auto app = QCoreApplication::instance();

    return
        (app->property(overrideFlagProperty).toBool()) ?
            app->property(overrideFileProperty).toString() :
            defaultPath();
}

void overridePath(const QString &path)
{
    auto app = QCoreApplication::instance();

    app->setProperty(overrideFlagProperty, true);
    app->setProperty(overrideFileProperty, path);
}

void initSchema(Database &database)
{
    QString dbSchemaVersion;

    auto query = database.execQuery(
        QStringLiteral("SELECT value FROM SchemaInfo WHERE key = 'version'"),
        /* ignore error */ true);

    if (query.next()) {
        dbSchemaVersion = query.value(0).toString();
    }

    // Early bail-out if the schema is up-to-date
    if (dbSchemaVersion == version()) {
        return;
    }

    // Transition to KF5:
    // We left the world of Nepomuk, and all the ontologies went
    // across the sea to the Undying Lands.
    // This needs to be done before executing the schema() queries
    // so that we do not create new (empty) tables and block these
    // queries from being executed.
    if (dbSchemaVersion < QStringLiteral("2014.04.14")) {
        database.execQuery(
            QStringLiteral("ALTER TABLE nuao_DesktopEvent RENAME TO ResourceEvent"),
            /* ignore error */ true);
        database.execQuery(
            QStringLiteral("ALTER TABLE kext_ResourceScoreCache RENAME TO ResourceScoreCache"),
            /* ignore error */ true);
    }

    database.execQueries(ResourcesDatabaseSchema::schema());

    // We can not allow empty fields for activity and agent, they need to
    // be at least magic values. These do not change the structure
    // of the database, but the old data.
    if (dbSchemaVersion < QStringLiteral("2015.02.09")) {
        const QString updateActivity =
            QStringLiteral("SET usedActivity=':global' "
            "WHERE usedActivity IS NULL OR usedActivity = ''");

        const QString updateAgent =
            QStringLiteral("SET initiatingAgent=':global' "
            "WHERE initiatingAgent IS NULL OR initiatingAgent = ''");

        // When the activity field was empty, it meant the file was
        // linked to all activities (aka :global)
        database.execQuery("UPDATE ResourceLink " + updateActivity);

        // When the agent field was empty, it meant the file was not
        // linked to a specified agent (aka :global)
        database.execQuery("UPDATE ResourceLink " + updateAgent);

        // These were not supposed to be empty, but in the case they were,
        // deal with them as well
        database.execQuery("UPDATE ResourceEvent " + updateActivity);
        database.execQuery("UPDATE ResourceEvent " + updateAgent);
        database.execQuery("UPDATE ResourceScoreCache " + updateActivity);
        database.execQuery("UPDATE ResourceScoreCache " + updateAgent);

    }
}

} // namespace Common
} // namespace ResourcesDatabaseSchema

