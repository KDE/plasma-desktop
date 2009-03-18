#include "kimtheme.h"
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

            KDesktopFile df(theme);
            QString name = df.readName();
            if (name.isEmpty()) {
                name = packageName;
            }
            themeNameList << name;
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
        } else if (!path.startsWith("/")) {
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

    Theme *q;

    QString themeName;

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
    connect(Plasma::Theme::defaultTheme(),SIGNAL(themeChanged()),SLOT(themeUpdated()));
    d->rescanThemes();
    setThemeName("default");
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
    int idx = d->themeNameList.indexOf(themeName);
    if (idx != -1) {
        d->themeName = themeName;
        KDesktopFile df(d->themeList[idx]);
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
        d->colors[StatusbarTextColor].setNamedColor(statusbarColors.readEntry(
                    "Text",defaultTextColorName));
        d->colors[StatusbarBackgroundColor].setNamedColor(statusbarColors.readEntry(
                    "TextBackground",defaultBackgroundColorName));

        KConfigGroup lookuptableColors(&colorConfig,"LookupTable:Color");
        d->colors[AuxilaryTextColor].setNamedColor(lookuptableColors.readEntry(
                    "AuxilaryText",defaultTextColorName));
        d->colors[AuxilaryBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                    "AuxilaryTextBackground",defaultBackgroundColorName));
        d->colors[PreeditTextColor].setNamedColor(lookuptableColors.readEntry(
                    "PreeditText",defaultTextColorName));
        d->colors[PreeditBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                    "PreeditTextBackground",defaultBackgroundColorName));
        d->colors[LookupTableLabelTextColor].setNamedColor(lookuptableColors.readEntry(
                    "LookupTableLabelText",defaultBackgroundColorName));
        d->colors[LookupTableLabelBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                    "LookupTableLabelBackground",defaultTextColorName));
        d->colors[LookupTableEntryTextColor].setNamedColor(lookuptableColors.readEntry(
                    "LookupTableEntryText",defaultTextColorName));
        d->colors[LookupTableEntryBackgroundColor].setNamedColor(lookuptableColors.readEntry(
                    "LookupTableEntryBackground",defaultBackgroundColorName));
        d->findRealPath(d->statusbarImagePath);
        d->findRealPath(d->statusbarLayoutPath);
        d->findRealPath(d->lookuptableImagePath);
        d->findRealPath(d->lookuptableLayoutPath);

        emit themeChanged();
    }
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
