/*
 *   Copyright 2014 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef LOOKANDFEELACCESS_H
#define LOOKANDFEELACCESS_H

#include <QtCore/QObject>

#include <KPluginInfo>
#include <Plasma/Package>

class LookAndFeelAccessPrivate;

class LookAndFeelAccess : public QObject
{
    Q_OBJECT

public:
    LookAndFeelAccess(QObject *parent = 0);
    ~LookAndFeelAccess();

    /**
     * Set a custom look and feel package that overrides the global one
     *
     * @param custom package name
     */
    void setTheme(const QString &theme);

    /**
     * @return custom look and feel package that overrides the global one
     */
    QString theme();

    /**
     * Get the path to a given file based on the key and an optional filename.
     * Example: finding the main script in a scripting package:
     *      filePath("mainscript")
     *
     * Example: finding a specific image in the images directory:
     *      filePath("images", "myimage.png")
     *
     * @param key the key of the file type to look for,
     * @param filename optional name of the file to locate within the package
     * @return path to the file on disk. QString() if not found.
     **/
    QString filePath(const char *key, const QString &filename = QString()) const;

    /**
     * @return the package metadata object.
     */
    KPluginInfo metadata() const;

    /**
     * Lists all available Look And Feel packages, if provided, will list only packages
     * that do provide a certain component, like the splashscreen, or the lockscreen etc.
     *
     * @param component the file we want packages to have
     * @return A list of the matching Plasma::Package instances
     */
    static QList<Plasma::Package> availablePackages(const QString &component = QString());

Q_SIGNALS:
    void themeChanged(const QString &theme);
    void packageChanged();

private:
    LookAndFeelAccessPrivate *const d;
    Q_PRIVATE_SLOT(d, void settingsFileChanged(QString))
};

#endif
