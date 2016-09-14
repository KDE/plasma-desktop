/*
 * Copyright © 2005-2007 Fredrik Höglund <fredrik@kde.org>
 * Copyright © 2016 Jason A. Donenfeld <jason@zx2c4.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>
#include <KShell>
#include <QStringList>
#include <QDir>
#include <QX11Info>

#include "thememodel.h"
#include "xcursortheme.h"

#include <X11/Xcursor/Xcursor.h>

CursorThemeModel::CursorThemeModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    insertThemes();
}

CursorThemeModel::~CursorThemeModel()
{
   qDeleteAll(list);
   list.clear();
}

void CursorThemeModel::refreshList()
{
    beginResetModel();
    qDeleteAll(list);
    list.clear();
    endResetModel();
    insertThemes();
}

QVariant CursorThemeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    // Only provide text for the headers
    if (role != Qt::DisplayRole)
        return QVariant();

    // Horizontal header labels
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case NameColumn:
                return i18n("Name");

            case DescColumn:
                return i18n("Description");

            default: return QVariant();
        }
    }

    // Numbered vertical header lables
    return QString(section);
}


QVariant CursorThemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= list.count())
        return QVariant();

    const CursorTheme *theme = list.at(index.row());

    // Text label
    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case NameColumn:
                return theme->title();

            case DescColumn:
                return theme->description();

            default: return QVariant();
        }
    }

    // Description for the first name column
    if (role == CursorTheme::DisplayDetailRole && index.column() == NameColumn)
        return theme->description();

    // Icon for the name column
    if (role == Qt::DecorationRole && index.column() == NameColumn)
        return theme->icon();

    return QVariant();
}


void CursorThemeModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column);
    Q_UNUSED(order);

    // Sorting of the model isn't implemented, as the KCM currently uses
    // a sorting proxy model.
}


const CursorTheme *CursorThemeModel::theme(const QModelIndex &index)
{
    if (!index.isValid())
        return NULL;

    if (index.row() < 0 || index.row() >= list.count())
        return NULL;

    return list.at(index.row());
}


QModelIndex CursorThemeModel::findIndex(const QString &name)
{
    uint hash = qHash(name);

    for (int i = 0; i < list.count(); i++)
    {
        const CursorTheme *theme = list.at(i);
        if (theme->hash() == hash)
            return index(i, 0);
    }

    return QModelIndex();
}


QModelIndex CursorThemeModel::defaultIndex()
{
    return findIndex(defaultName);
}


const QStringList CursorThemeModel::searchPaths()
{
    if (!baseDirs.isEmpty())
        return baseDirs;

    baseDirs = QString(XcursorLibraryPath()).split(':', QString::SkipEmptyParts);
    std::transform(baseDirs.begin(), baseDirs.end(), baseDirs.begin(), KShell::tildeExpand);
    baseDirs.removeDuplicates();

    return baseDirs;
}


bool CursorThemeModel::hasTheme(const QString &name) const
{
    const uint hash = qHash(name);

    foreach (const CursorTheme *theme, list)
        if (theme->hash() == hash)
            return true;

    return false;
}


bool CursorThemeModel::isCursorTheme(const QString &theme, const int depth)
{
    // Prevent infinite recursion
    if (depth > 10)
        return false;

    // Search each icon theme directory for 'theme'
    foreach (const QString &baseDir, searchPaths())
    {
        QDir dir(baseDir);
        if (!dir.exists() || !dir.cd(theme))
            continue;

        // If there's a cursors subdir, we'll assume this is a cursor theme
        if (dir.exists(QStringLiteral("cursors")))
            return true;

        // If the theme doesn't have an index.theme file, it can't inherit any themes.
        if (!dir.exists(QStringLiteral("index.theme")))
            continue;

        // Open the index.theme file, so we can get the list of inherited themes
        KConfig config(dir.path() + "/index.theme", KConfig::NoGlobals);
        KConfigGroup cg(&config, "Icon Theme");

        // Recurse through the list of inherited themes, to check if one of them
        // is a cursor theme.
        QStringList inherits = cg.readEntry("Inherits", QStringList());
        foreach (const QString &inherit, inherits)
        {
            // Avoid possible DoS
            if (inherit == theme)
                continue;

            if (isCursorTheme(inherit, depth + 1))
                return true;
        }
    }

    return false;
}


bool CursorThemeModel::handleDefault(const QDir &themeDir)
{
    QFileInfo info(themeDir.path());

    // If "default" is a symlink
    if (info.isSymLink())
    {
        QFileInfo target(info.symLinkTarget());
        if (target.exists() && (target.isDir() || target.isSymLink()))
            defaultName = target.fileName();

        return true;
    }

    // If there's no cursors subdir, or if it's empty
    if (!themeDir.exists(QStringLiteral("cursors")) || QDir(themeDir.path() + "/cursors")
          .entryList(QDir::Files | QDir::NoDotAndDotDot ).isEmpty())
    {
        if (themeDir.exists(QStringLiteral("index.theme")))
        {
            XCursorTheme theme(themeDir);
            if (!theme.inherits().isEmpty())
                defaultName = theme.inherits().at(0);
        }
        return true;
    }

    defaultName = QStringLiteral("default");
    return false;
}


void CursorThemeModel::processThemeDir(const QDir &themeDir)
{
    bool haveCursors = themeDir.exists(QStringLiteral("cursors"));

    // Special case handling of "default", since it's usually either a
    // symlink to another theme, or an empty theme that inherits another
    // theme.
    if (defaultName.isNull() && themeDir.dirName() == QLatin1String("default"))
    {
        if (handleDefault(themeDir))
            return;
    }

    // If the directory doesn't have a cursors subdir and lacks an
    // index.theme file it can't be a cursor theme.
    if (!themeDir.exists(QStringLiteral("index.theme")) && !haveCursors)
        return;

    // Create a cursor theme object for the theme dir
    XCursorTheme *theme = new XCursorTheme(themeDir);

    // Skip this theme if it's hidden.
    if (theme->isHidden()) {
        delete theme;
        return;
    }

    // If there's no cursors subdirectory we'll do a recursive scan
    // to check if the theme inherits a theme with one.
    if (!haveCursors)
    {
        bool foundCursorTheme = false;

        foreach (const QString &name, theme->inherits())
            if ((foundCursorTheme = isCursorTheme(name)))
                break;

        if (!foundCursorTheme) {
            delete theme;
            return;
        }
    }

    // Append the theme to the list
    beginInsertRows(QModelIndex(), list.size(), list.size());
    list.append(theme);
    endInsertRows();
}


void CursorThemeModel::insertThemes()
{
    // Scan each base dir for Xcursor themes and add them to the list.
    foreach (const QString &baseDir, searchPaths())
    {
        QDir dir(baseDir);
        if (!dir.exists())
            continue;

        // Process each subdir in the directory
        foreach (const QString &name, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            // Don't process the theme if a theme with the same name already exists
            // in the list. Xcursor will pick the first one it finds in that case,
            // and since we use the same search order, the one Xcursor picks should
            // be the one already in the list.
            if (hasTheme(name) || !dir.cd(name))
                continue;

            processThemeDir(dir);
            dir.cdUp(); // Return to the base dir
        }
    }

    // The theme Xcursor will end up using if no theme is configured
    if (defaultName.isNull() || !hasTheme(defaultName))
        defaultName = QStringLiteral("KDE_Classic");
}


bool CursorThemeModel::addTheme(const QDir &dir)
{
    XCursorTheme *theme = new XCursorTheme(dir);

    // Don't add the theme to the list if it's hidden
    if (theme->isHidden()) {
        delete theme;
        return false;
    }

    // ### If the theme is hidden, the user will probably find it strange that it
    //     doesn't appear in the list view. There also won't be a way for the user
    //     to delete the theme using the KCM. Perhaps a warning about this should
    //     be issued, and the user be given a chance to undo the installation.

    // If an item with the same name already exists in the list,
    // we'll remove it before inserting the new one.
    for (int i = 0; i < list.count(); i++)
    {
        if (list.at(i)->hash() == theme->hash()) {
            removeTheme(index(i, 0));
            break;
        }
    }

    // Append the theme to the list
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    list.append(theme);
    endInsertRows();

    return true;
}


void CursorThemeModel::removeTheme(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    beginRemoveRows(QModelIndex(), index.row(), index.row());
    delete list.takeAt(index.row());
    endRemoveRows();
}

