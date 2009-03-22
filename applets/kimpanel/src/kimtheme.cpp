/***************************************************************************
 *   Copyright (C) 2009 by Wang Hoi <zealot.hoi@gmail.com>                 *
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

#include "kimtheme.h"
#include "kimpanelsettings.h"
#include <plasma/theme.h>
#include <KDesktopFile>
#include <KStandardDirs>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

namespace KIM
{

class ThemePrivate
{
public:
    ThemePrivate(Theme *theme)
        :q(theme)
    {
    }

    void rescanThemes()
    {
        KStandardDirs dirs;
        themeList = dirs.findAllResources("data", 
               "kimpanel/themes/*/metadata.desktop",
               KStandardDirs::NoDuplicates);
        themeNameList.clear();
        foreach (const QString &theme, themeList) {
            int themeSepIndex = theme.lastIndexOf('/', -1);
            QString themeRoot = theme.left(themeSepIndex);
            int themeNameSepIndex = themeRoot.lastIndexOf('/', -1);
            QString packageName = themeRoot.right(themeRoot.length() - themeNameSepIndex - 1);
            themeNameList << packageName;
        }
    }
    void findRealPath(QString &path)
    {
        if (path.startsWith("default:/")) {
            int idx = themeNameList.indexOf("default");
            QString defaultTheme = themeList[idx];
            QString defaultPrefix = defaultTheme.left(defaultTheme.lastIndexOf("/")+1);
            path.replace(0,path.indexOf("/")+1,defaultPrefix);
        } else if (path.startsWith("plasma:/")) {
            path.remove(0,path.indexOf("/")+1);
            path = Plasma::Theme::defaultTheme()->imagePath(path);
        } else if (!path.startsWith('/')) {
            int idx = themeNameList.indexOf(themeName);
            QString theme = themeList[idx];
            QString prefix = theme.left(theme.lastIndexOf("/")+1);
            path.insert(0,prefix);
        }
        kDebug() << path;
    }

    void themeUpdated()
    {
        q->setThemeName(themeName);
    }

    void configUpdated()
    {
        q->setThemeName(Settings::self()->theme());
    }

    Theme *q;

    QString themeName;
    QString themePath;

    QString colorPath;
    QString statusbarImagePath;
    QString statusbarLayoutPath;
    QString lookuptableImagePath;
    QString lookuptableLayoutPath;

    QMap<Theme::ColorRole, QColor> colors;
    QMap<Theme::FontRole, QFont> fonts;

    QStringList themeList;
    QStringList themeNameList;
};

    Theme::Theme(QObject *parent)
:QObject(parent),
    d(new ThemePrivate(this))
{
    //connect(Plasma::Theme::defaultTheme(),SIGNAL(themeChanged()),this,SLOT(themeUpdated()));
    connect(KIM::Settings::self(),SIGNAL(configChanged()),this,SLOT(configUpdated()));

    setThemeName(Settings::self()->theme());
}

Theme::~Theme()
{

}

class ThemeSingleton
{
public:
    ThemeSingleton()
    {
        //self.d->isDefault = true;
    }

    Theme self;
};

K_GLOBAL_STATIC(ThemeSingleton, privateThemeSelf)

Theme *Theme::defaultTheme()
{
    return &privateThemeSelf->self;
}

bool Theme::isValid() const
{
    return d->themeNameList.contains(d->themeName);
}

void Theme::setThemeName(const QString &themeName)
{
    d->rescanThemes();
    kDebug() << themeName << d->themeNameList;
    int idx = d->themeNameList.indexOf(themeName);
    if (idx == -1) {
        idx = 0;
    }
    d->themeName = d->themeNameList[idx];
    d->themePath = d->themeList[idx];

    KDesktopFile df(d->themePath);
    KConfigGroup cg(&df,"Desktop Entry");
    d->colorPath = cg.readEntry("X-KIMPanel-Theme-Color","default:/colors");
    d->statusbarImagePath = cg.readEntry("X-KIMPanel-Theme-StatusbarImage","plasma:/dialogs/panel-background");
    d->statusbarLayoutPath = cg.readEntry("X-KIMPanel-Theme-StatusbarLayout","default:/frame.js");
    d->lookuptableImagePath = cg.readEntry("X-KIMPanel-Theme-LookupTableImage","plasma:/dialogs/background");
    d->lookuptableLayoutPath = cg.readEntry("X-KIMPanel-Theme-StatusbarLayout","default:/frame.js");

    d->findRealPath(d->colorPath);
    KConfig colorConfig(d->colorPath);

    KConfigGroup statusbarColors(&colorConfig,"Statusbar:Color");
    QString defaultTextColorName = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor).name();
    QString defaultBackgroundColorName = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor).name();
    QString defaultTransparentBackgroundColorName = "transparent";
    d->colors[StatusbarTextColor].setNamedColor(statusbarColors.readEntry(
                "Text",defaultTextColorName));
    d->colors[StatusbarBackgroundColor].setNamedColor(statusbarColors.readEntry(
                "TextBackground",defaultTransparentBackgroundColorName));

    KConfigGroup lookuptableColors(&colorConfig,"LookupTable:Color");
    d->colors[AuxiliaryTextColor].setNamedColor(lookuptableColors.readEntry(
                "AuxiliaryText",defaultTextColorName));
    d->colors[AuxiliaryBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                "AuxiliaryTextBackground",defaultTransparentBackgroundColorName));
    d->colors[PreeditTextColor].setNamedColor(lookuptableColors.readEntry(
                "PreeditText",defaultTextColorName));
    d->colors[PreeditBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                "PreeditTextBackground",defaultTransparentBackgroundColorName));
    d->colors[LookupTableLabelTextColor].setNamedColor(lookuptableColors.readEntry(
                "LookupTableLabelText",defaultBackgroundColorName));
    d->colors[LookupTableLabelBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                "LookupTableLabelBackground",defaultTextColorName));
    d->colors[LookupTableEntryTextColor].setNamedColor(lookuptableColors.readEntry(
                "LookupTableEntryText",defaultTextColorName));
    d->colors[LookupTableEntryBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                "LookupTableEntryBackground",defaultTransparentBackgroundColorName));
    d->findRealPath(d->statusbarImagePath);
    d->findRealPath(d->statusbarLayoutPath);
    d->findRealPath(d->lookuptableImagePath);
    d->findRealPath(d->lookuptableLayoutPath);

    emit themeChanged();
}

QString Theme::themeName() const
{
    return d->themeName;
}

QStringList Theme::listThemeNames() const
{
    return d->themeNameList;
}

QString Theme::statusbarImagePath() const
{
    return d->statusbarImagePath;
}

QString Theme::statusbarLayoutPath() const
{
    return d->statusbarLayoutPath;
}

QString Theme::lookuptableImagePath() const
{
    return d->lookuptableImagePath;
}

QString Theme::lookuptableLayoutPath() const
{
    return d->lookuptableLayoutPath;
}

QColor Theme::color(ColorRole role) const
{
    return d->colors[role];
}

QFont Theme::font(FontRole role) const
{
    return d->fonts[role];
}

QFontMetrics Theme::fontMetrics(FontRole role) const
{
    return QFontMetrics(font(role));
}

} // namespace KIM

#include "kimtheme.moc"
// vim: sw=4 sts=4 et tw=100
