/*
 * Copyright 2020 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QStandardPaths>
#include <QDir>
#include <QUrl>
#include <QDebug>

#include <KIO/DeleteJob>

#include "gtkthemesmodel.h"

GtkThemesModel::GtkThemesModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_selectedTheme(QStringLiteral("Breeze"))
    , m_themesList()
{

}

void GtkThemesModel::load()
{
    QMap<QString, QString> gtk3ThemesNames;

    static const QStringList gtk3SubdirPattern(QStringLiteral("gtk-3.*"));
    for (const QString &possibleThemePath : possiblePathsToThemes()) {
        // If the directory contains any of gtk-3.X folders, it is the GTK3 theme for sure
        QDir possibleThemeDirectory(possibleThemePath);
        if (!possibleThemeDirectory.entryList(gtk3SubdirPattern, QDir::Dirs).isEmpty()) {

            // Do not show dark Breeze GTK variant, since the colors of it
            // are coming from the color scheme and selecting them here
            // is redundant and does not work
            if (possibleThemeDirectory.dirName() == QStringLiteral("Breeze-Dark")) {
                continue;
            }

            gtk3ThemesNames.insert(possibleThemeDirectory.dirName(), possibleThemeDirectory.path());
        }
    }

    setThemesList(gtk3ThemesNames);
}

QString GtkThemesModel::themePath(const QString &themeName) {
    if (themeName.isEmpty()) {
        return QString();
    } else {
        return m_themesList.find(themeName).value();
    }
}

QVariant GtkThemesModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole || role == Roles::ThemeNameRole) {
        if (index.row() < 0 || index.row() > m_themesList.count()) {
            return QVariant();
        }

        return m_themesList.keys().at(index.row());
    } else if (role == Roles::ThemePathRole) {
        if (index.row() < 0 || index.row() > m_themesList.count()) {
            return QVariant();
        }

        return m_themesList.values().at(index.row());
    } else {
        return QVariant();
    }
}

QHash<int, QByteArray> GtkThemesModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles[Roles::ThemeNameRole] = "theme-name";
    roles[Roles::ThemePathRole] = "theme-path";

    return roles;
}

int GtkThemesModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_themesList.count();
}

void GtkThemesModel::setThemesList(const QMap<QString, QString>& themes)
{
    beginResetModel();
    m_themesList = themes;
    endResetModel();
}

QMap<QString, QString> GtkThemesModel::themesList() {
    return m_themesList;
}

void GtkThemesModel::setSelectedTheme(const QString &themeName)
{
    m_selectedTheme = themeName;
}

QString GtkThemesModel::selectedTheme()
{
    return m_selectedTheme;
}

QStringList GtkThemesModel::possiblePathsToThemes()
{
    QStringList possibleThemesPaths;

    QStringList themesLocationsPaths = QStandardPaths::locateAll(
            QStandardPaths::GenericDataLocation,
            QStringLiteral("themes"),
            QStandardPaths::LocateDirectory);
    themesLocationsPaths << QDir::homePath() + QStringLiteral("/.themes");

    for (const QString& themesLocationPath : themesLocationsPaths) {
        QStringList possibleThemesDirectoriesNames = QDir(themesLocationPath).entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
        for (const QString &possibleThemeDirectoryName : possibleThemesDirectoriesNames) {
            possibleThemesPaths += themesLocationPath + '/' + possibleThemeDirectoryName;
        }
    }

    return possibleThemesPaths;
}

bool GtkThemesModel::selectedThemeRemovable()
{
    return themePath(m_selectedTheme).contains(QDir::homePath());
}

void GtkThemesModel::removeSelectedTheme()
{
    QString path = themePath(m_selectedTheme);
    KIO::DeleteJob* deleteJob = KIO::del(QUrl::fromLocalFile(path), KIO::HideProgressInfo);
    connect(deleteJob, &KJob::finished, this, [this](){
        Q_EMIT themeRemoved();
    });
}

int GtkThemesModel::findThemeIndex(const QString &themeName)
{
    return static_cast<int>(std::distance(m_themesList.begin(), m_themesList.find(themeName)));
}

void GtkThemesModel::setSelectedThemeDirty()
{
    Q_EMIT selectedThemeChanged(m_selectedTheme);
}
