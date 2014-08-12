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


#include "lookandfeelaccess.h"

#include "shellpluginloader.h"

#include <QDebug>
#include <QDir>
#include <KDirWatch>
#include <KSharedConfig>

class LookAndFeelAccessPrivate
{
public:
    LookAndFeelAccessPrivate(LookAndFeelAccess *access)
        : q(access)
    {
    }

    void settingsFileChanged();


    LookAndFeelAccess *q;
    Plasma::Package package;
    Plasma::Package defaultPackage;
    Plasma::Package themePackage;
    QString theme;
    KSharedConfig::Ptr config;
};

void LookAndFeelAccessPrivate::settingsFileChanged()
{
    config->reparseConfiguration();
    KConfigGroup cg(config, "KDE");

    const QString packageName = cg.readEntry("LookAndFeelPackage", QString());
    const QString oldPackageName = package.isValid() ? package.metadata().pluginName() : QString();

    if (packageName == oldPackageName) {
        return;
    }

    //Invalid
    if (packageName.isEmpty() && !oldPackageName.isEmpty()) {
        package = Plasma::Package();
    } else if (!packageName.isEmpty()) {
        package = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
        package.setPath(packageName);
    }

    emit q->packageChanged();
}

LookAndFeelAccess::LookAndFeelAccess(QObject *parent)
    : QObject(parent),
      d(new LookAndFeelAccessPrivate(this))
{
    ShellPluginLoader::init();
    d->defaultPackage = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
    d->defaultPackage.setPath("org.kde.lookandfeel");

    d->config = KSharedConfig::openConfig("kdeglobals");
    KConfigGroup cg(d->config, "KDE");
    const QString packageName = cg.readEntry("LookAndFeelPackage", QString());
    if (!packageName.isEmpty()) {
        d->package = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
        d->package.setPath(packageName);
    }

    const QString configFile = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/kdeglobals";
    KDirWatch::self()->addFile(configFile);

    // Catch both, direct changes to the config file ...
    connect(KDirWatch::self(), SIGNAL(dirty()), this, SLOT(settingsFileChanged()));
    // ... but also remove/recreate cycles, like KConfig does it
    connect(KDirWatch::self(), SIGNAL(created()), this, SLOT(settingsFileChanged()));
}

LookAndFeelAccess::~LookAndFeelAccess()
{
}

void LookAndFeelAccess::setTheme(const QString &theme)
{
    if (theme == d->theme) {
        return;
    }

    d->theme = theme;
    d->themePackage = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
    d->themePackage.setPath(theme);

    emit themeChanged(theme);
}

QString LookAndFeelAccess::theme()
{
    return d->theme;
}

QString LookAndFeelAccess::filePath(const char *key, const QString &filename) const
{
    QString path = d->themePackage.filePath(key, filename);

    if (!path.isEmpty()) {
        return path;
    }

    path = d->package.filePath(key, filename);

    if (!path.isEmpty()) {
        return path;
    } else {
        return d->defaultPackage.filePath(key, filename);
    }
}

KPluginInfo LookAndFeelAccess::metadata() const
{
    if (d->package.isValid()) {
        return d->package.metadata();
    } else {
        return d->defaultPackage.metadata();
    }
}

QList<Plasma::Package> LookAndFeelAccess::availablePackages(const QString &component)
{
    QList<Plasma::Package> packages;
    QStringList paths;
    QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (const QString &path : dataPaths) {
        QDir dir(path + "/plasma/look-and-feel");
        paths << dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }

    for (const QString &path : paths) {
        Plasma::Package pkg = Plasma::PluginLoader::self()->loadPackage("Plasma/LookAndFeel");
        pkg.setPath(path);
        if (component.isEmpty() || !pkg.filePath(component.toUtf8()).isEmpty()) {
            packages << pkg;
        }
    }

    return packages;
}

#include "moc_lookandfeelaccess.cpp"
