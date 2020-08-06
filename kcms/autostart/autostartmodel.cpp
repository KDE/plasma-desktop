/***************************************************************************
 *   Copyright (C) 2020 by Méven Car <meven.car@enioka.com>                *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/
#include "autostartmodel.h"

#include <QStandardPaths>
#include <QDir>
#include <KDesktopFile>
#include <KShell>
#include <KConfigGroup>
#include <QDebug>

#include <KLocalizedString>
#include <KIO/DeleteJob>
#include <KIO/CopyJob>
#include <KIO/JobUiDelegate>
#include <KJobWidgets>
#include <KService>


namespace {
    // FDO user autostart directories are
    // .config/autostart which has .desktop files executed by klaunch

    //Then we have Plasma-specific locations which run scripts
    // .config/autostart-scripts which has scripts executed by ksmserver
    // .config/plasma-workspace/shutdown which has scripts executed by startkde
    // .config/plasma-workspace/env which has scripts executed by startkde

    //in the case of pre-startup they have to end in .sh
    //everywhere else it doesn't matter

    //the comment above describes how autostart *currently* works, it is not definitive documentation on how autostart *should* work

    // share/autostart shouldn't be an option as this should be reserved for global autostart entries

// must match enum AutostartEntrySource
QStringList autostartPaths()
{
    return {
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/autostart/"),
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/autostart-scripts/"),
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/plasma-workspace/shutdown/"),
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/plasma-workspace/env/")
    };
}
Q_GLOBAL_STATIC_WITH_ARGS(QStringList, s_paths, (autostartPaths()))

QStringList autoStartPathNames()
{
    return {
        i18n("Startup"),
        i18n("Logout"),
        i18n("Before session startup")
    };
}
Q_GLOBAL_STATIC_WITH_ARGS(QStringList, s_pathName, (autoStartPathNames()))

static bool checkEntry(const AutostartEntry &entry)
{
    QStringList commandLine = KShell::splitArgs(entry.command);
    if (commandLine.isEmpty()) {
        return false;
    }

    const QString exe = commandLine.first();
    if (exe.isEmpty() || QStandardPaths::findExecutable(exe).isEmpty()) {
        return false;
    }
    return true;
}

static AutostartEntry loadDesktopEntry(const QString &fileName)
{
    KDesktopFile config(fileName);
    const KConfigGroup grp = config.desktopGroup();
    const auto name = config.readName();
    const auto command = grp.readEntry("Exec");

    const bool hidden = grp.readEntry("Hidden", false);
    const QStringList notShowList = grp.readXdgListEntry("NotShowIn");
    const QStringList onlyShowList = grp.readXdgListEntry("OnlyShowIn");
    const bool enabled = !(hidden ||
                           notShowList.contains(QLatin1String("KDE")) ||
                           (!onlyShowList.isEmpty() && !onlyShowList.contains(QLatin1String("KDE"))));

    const auto lstEntry = grp.readXdgListEntry("OnlyShowIn");
    const bool onlyInPlasma = lstEntry.contains(QLatin1String("KDE"));

    return {
        name,
        command,
        AutostartEntrySource::XdgAutoStart, // .config/autostart load desktop at startup
        enabled,
        fileName,
        onlyInPlasma
    };
}
}

AutostartEntrySource AutostartModel::sourceFromInt(int index)
{
    switch (index) {
    case XdgAutoStart:
    case XdgScripts:
    case PlasmaShutdown:
    case PlasmaStart:
        return static_cast<AutostartEntrySource>(index);
    default:
        Q_UNREACHABLE();
    }
    return XdgAutoStart;
}

AutostartModel::AutostartModel(QWidget *parent)
    : QAbstractListModel(parent)
{
    m_window = parent;
}

QString AutostartModel::XdgAutoStartPath()
{
    return s_paths->at(0);
}

QStringList AutostartModel::listPath()
{
    return *s_paths;
}

QStringList AutostartModel::listPathName()
{
    return *s_pathName;
}

void AutostartModel::load()
{
    beginResetModel();

    m_entries.clear();

    QDir autostartdir(XdgAutoStartPath());
    if (!autostartdir.exists()) {
        autostartdir.mkpath(XdgAutoStartPath());
    }

    autostartdir.setFilter(QDir::Files);

    const auto filesInfo = autostartdir.entryInfoList();
    for (const QFileInfo &fi : filesInfo) {
        QString filename = fi.fileName();
        bool desktopFile = filename.endsWith(QLatin1String(".desktop"));
        if (desktopFile) {
            AutostartEntry entry = loadDesktopEntry(fi.absoluteFilePath());

            if (!checkEntry(entry)) {
                continue;
            }

            m_entries.push_back(entry);
        }
    }

    // add scripts, skips first value of listPath
    // .config/autostart that is not compatible with scripts
    for (int i = 1; i < listPath().length(); i++) {
        const auto path = listPath().at(i);
        const AutostartEntrySource kind = AutostartModel::sourceFromInt(i);

        QDir dir(path);
        if (!dir.exists()) {
            dir.mkpath(path);
        }

        dir.setFilter(QDir::Files);

        const auto autostartDirFilesInfo = dir.entryInfoList();
        for (const QFileInfo &fi : autostartDirFilesInfo) {

            QString fileName = fi.absoluteFilePath();
            const bool isSymlink = fi.isSymLink();
            if (isSymlink) {
                fileName = fi.symLinkTarget();
            }

            m_entries.push_back({
                                    fi.fileName(),
                                    isSymlink ? fileName : "",
                                    kind,
                                    true,
                                    fi.absoluteFilePath(),
                                    false
                                });
        }
    }

    endResetModel();
}

int AutostartModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_entries.count();
}

bool AutostartModel::reloadEntry(const QModelIndex &index, const QString &fileName)
{
    if (!checkIndex(index)) {
        return false;
    }

    AutostartEntry newEntry = loadDesktopEntry(fileName);
    if (!checkEntry(newEntry)) {
        return false;
    }

    m_entries.replace(index.row(), newEntry);
    return true;
}

QVariant AutostartModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return QVariant();
    }

    const auto &entry = m_entries.at(index.row());

    switch (role) {
    case Qt::DisplayRole: return entry.name;
    case Command: return entry.command;
    case Enabled: return entry.enabled;
    case Source: return entry.source;
    case FileName: return entry.fileName;
    case OnlyInPlasma: return entry.onlyInPlasma;
    }

    return QVariant();
}

bool AutostartModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!checkIndex(index)) {
        return false;
    }

    bool dirty = false;
    AutostartEntry &entry = m_entries[index.row()];

    switch (role) {
    case Qt::EditRole: {
        if (entry.name == value.toString()) {
            return false;
        }
        entry.name = value.toString();
        dirty = true;
        break;
    }
    case Command: {
        if (entry.command == value.toString()) {
            return false;
        }
        entry.command = value.toString();
        dirty = true;
        break;
    }
    case Enabled: {
        if (entry.enabled == value.toBool()) {
            return false;
        }
        entry.enabled = value.toBool();

        KDesktopFile kc(entry.fileName);
        KConfigGroup grp = kc.desktopGroup();
        if ( grp.hasKey( "Hidden" ) && entry.enabled) {
            grp.deleteEntry( "Hidden" );
        } else {
            grp.writeEntry("Hidden", !entry.enabled);
        }
        kc.sync();

        dirty = true;
        break;
    }
    case OnlyInPlasma: {
        if (value == entry.onlyInPlasma) {
            return false;
        }
        entry.onlyInPlasma = value.toBool();

        KDesktopFile kc(entry.fileName);
        KConfigGroup grp = kc.desktopGroup();
        auto lstEntry = grp.readXdgListEntry("OnlyShowIn");
        if (lstEntry.contains(QLatin1String("KDE") ) && !entry.onlyInPlasma ) {
            lstEntry.removeAll( QStringLiteral("KDE") );
        } else if (!lstEntry.contains(QLatin1String("KDE") ) && entry.onlyInPlasma ) {
            lstEntry.append( QStringLiteral("KDE") );
        }
        grp.writeXdgListEntry( "OnlyShowIn", lstEntry );
        kc.sync();

        dirty = true;
        break;
    }
    case Source: {
        if (entry.source ==  XdgAutoStart || (entry.source == value.toInt())) {
            return false;
        }

        AutostartEntrySource newSource = AutostartModel::sourceFromInt(value.toInt());
        auto path = listPath().at(newSource);

        QUrl newFileName = QUrl::fromLocalFile(entry.fileName);
        newFileName.setPath( path + newFileName.fileName());
        KIO::CopyJob *job = KIO::move(QUrl::fromLocalFile(entry.fileName), newFileName, KIO::HideProgressInfo);
        KJobWidgets::setWindow(job, m_window);
        connect(job, &KIO::CopyJob::renamed, this, [&newFileName](KIO::Job * job, const QUrl & from, const QUrl & to) {
            Q_UNUSED(job)
            Q_UNUSED(from)

            // in case the destination filename had to be renamed
            newFileName = to;
        });

        if (job->exec()) {

            entry.source = newSource;

            entry.name = newFileName.fileName();
            entry.fileName = newFileName.path();
            dirty = true;
        }
        break;
    }
    }

    if (dirty) {
        if (role == Qt::EditRole) {
            emit dataChanged(index, index, {role, Qt::DisplayRole});
        } else {
            emit dataChanged(index, index, {role});
        }
    }

    return dirty;
}

bool AutostartModel::addEntry(const KService::Ptr &service)
{
    QString desktopPath;
    // It is important to ensure that we make an exact copy of an existing
    // desktop file (if selected) to enable users to override global autostarts.
    // Also see
    // https://bugs.launchpad.net/ubuntu/+source/kde-workspace/+bug/923360
    if (service->desktopEntryName().isEmpty() || service->entryPath().isEmpty()) {
        // create a new desktop file in s_desktopPath
        desktopPath = XdgAutoStartPath() + service->name() + QStringLiteral(".desktop");

        KDesktopFile desktopFile(desktopPath);
        KConfigGroup kcg = desktopFile.desktopGroup();
        kcg.writeEntry("Name", service->name());
        kcg.writeEntry("Exec", service->exec());
        kcg.writeEntry("Icon", "system-run");
        kcg.writeEntry("Path", "");
        kcg.writeEntry("Terminal", service->terminal() ? "True": "False");
        kcg.writeEntry("Type", "Application");
        desktopFile.sync();

    } else {
        desktopPath = XdgAutoStartPath() + service->desktopEntryName() + QStringLiteral(".desktop");

        // copy original desktop file to new path
        KDesktopFile desktopFile(service->entryPath());
        auto newDeskTopFile = desktopFile.copyTo(desktopPath);
        newDeskTopFile->sync();
    }

    const auto entry = AutostartEntry{
        service->name(),
        service->exec(),
        AutostartEntrySource:: XdgAutoStart, // .config/autostart load desktop at startup
        true,
        desktopPath,
        false
    };

    // push before the script items
    int index = 0;
    for (const AutostartEntry &e : qAsConst(m_entries)) {
        if (e.source == XdgAutoStart) {
            index++;
        } else {
            break;
        }
    }

    beginInsertRows(QModelIndex(), index, index);

    m_entries.insert(index, entry);

    endInsertRows();

    return true;
}

bool AutostartModel::addEntry(const QUrl &path, const bool isSymlink)
{
    beginInsertRows(QModelIndex(), m_entries.count(), m_entries.count());

    QString fileName = path.fileName();
    QUrl destinationScript = QUrl::fromLocalFile(listPath()[AutostartEntrySource::XdgScripts] + fileName);
    KIO::CopyJob *job;
    if (isSymlink) {
        job = KIO::link(path, destinationScript, KIO::HideProgressInfo);
    } else {
        job = KIO::copy(path, destinationScript, KIO::HideProgressInfo);
    }
    KJobWidgets::setWindow(job, m_window);
    connect(job, &KIO::CopyJob::renamed, this, [&destinationScript](KIO::Job * job, const QUrl & from, const QUrl & to) {
        Q_UNUSED(job)
        Q_UNUSED(from)

        // in case the destination filename had to be renamed
        destinationScript = to;
    });

    if (!job->exec()) {
        return false;
    }

    AutostartEntry entry = AutostartEntry{
        destinationScript.fileName(),
        isSymlink ? path.path() : "",
        AutostartEntrySource::XdgScripts, // .config/autostart load desktop at startup
        true,
        destinationScript.path(),
        false
    };

     m_entries.push_back(entry);
     endInsertRows();

     return true;
}

bool AutostartModel::removeEntry(const QModelIndex &index)
{
    auto entry = m_entries.at(index.row());

    KIO::DeleteJob* job = KIO::del(QUrl::fromLocalFile(entry.fileName), KIO::HideProgressInfo);

    if (job->exec()) {
        beginRemoveRows(QModelIndex(), index.row(), index.row());

        m_entries.remove(index.row());

        endRemoveRows();
        return true;
    }
    return false;
}

QHash<int, QByteArray> AutostartModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();

    roleNames.insert(Command, QByteArrayLiteral("command"));
    roleNames.insert(Enabled, QByteArrayLiteral("enabled"));
    roleNames.insert(Source, QByteArrayLiteral("source"));
    roleNames.insert(FileName, QByteArrayLiteral("fileName"));
    roleNames.insert(OnlyInPlasma, QByteArrayLiteral("onlyInPlasma"));

    return roleNames;
}
