/* This file is part of the KDE Project
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2019 Cyril Rossi <cyril.rossi@enioka.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcm.h"
#include "../krdb/krdb.h"
#include "config-kcm.h"
#include "config-workspace.h"

#include <KAboutData>
#include <KSharedConfig>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KIO/ApplicationLauncherJob>
#include <KAutostart>
#include <KDialogJobUiDelegate>
#include <KService>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>
#include <QQuickItem>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QProcess>
#include <QStandardItemModel>
#include <QX11Info>
#include <QStyle>
#include <QStyleFactory>

#include <KLocalizedString>
#include <KPackage/PackageLoader>

#include <X11/Xlib.h>

#include <updatelaunchenvjob.h>

#include "lookandfeelsettings.h"
#include "lookandfeeldata.h"

#ifdef HAVE_XCURSOR
#   include "../cursortheme/xcursor/xcursortheme.h"
#   include <X11/Xcursor/Xcursor.h>
#endif

#ifdef HAVE_XFIXES
#  include <X11/extensions/Xfixes.h>
#endif

KCMLookandFeel::KCMLookandFeel(QObject *parent, const QVariantList &args)
    : KQuickAddons::ManagedConfigModule(parent, args)
    , m_data(new LookAndFeelData(this))
    , m_config(QStringLiteral("kdeglobals"))
    , m_configGroup(m_config.group("KDE"))
    , m_applyColors(true)
    , m_applyWidgetStyle(true)
    , m_applyIcons(true)
    , m_applyPlasmaTheme(true)
    , m_applyCursors(true)
    , m_applyWindowSwitcher(true)
    , m_applyDesktopSwitcher(true)
    , m_resetDefaultLayout(false)
    , m_applyWindowDecoration(true)
{
    qmlRegisterType<LookAndFeelSettings>();
    qmlRegisterType<QStandardItemModel>();
    qmlRegisterType<KCMLookandFeel>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_lookandfeel"), i18n("Global Theme"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Marco Martin"), QString(), QStringLiteral("mart@kde.org"));
    setAboutData(about);
    setButtons(Apply | Default);

    m_model = new QStandardItemModel(this);
    QHash<int, QByteArray> roles = m_model->roleNames();
    roles[PluginNameRole] = "pluginName";
    roles[DescriptionRole] = "description";
    roles[ScreenshotRole] = "screenshot";
    roles[FullScreenPreviewRole] = "fullScreenPreview";
    roles[HasSplashRole] = "hasSplash";
    roles[HasLockScreenRole] = "hasLockScreen";
    roles[HasRunCommandRole] = "hasRunCommand";
    roles[HasLogoutRole] = "hasLogout";

    roles[HasColorsRole] = "hasColors";
    roles[HasWidgetStyleRole] = "hasWidgetStyle";
    roles[HasIconsRole] = "hasIcons";
    roles[HasPlasmaThemeRole] = "hasPlasmaTheme";
    roles[HasCursorsRole] = "hasCursors";
    roles[HasWindowSwitcherRole] = "hasWindowSwitcher";
    roles[HasDesktopSwitcherRole] = "hasDesktopSwitcher";
    m_model->setItemRoleNames(roles);
    loadModel();
}

KCMLookandFeel::~KCMLookandFeel()
{
}

void KCMLookandFeel::reloadModel()
{
    loadModel();
}

QStandardItemModel *KCMLookandFeel::lookAndFeelModel() const
{
    return m_model;
}

int KCMLookandFeel::pluginIndex(const QString &pluginName) const
{
    const auto results = m_model->match(m_model->index(0, 0), PluginNameRole, pluginName);
    if (results.count() == 1) {
        return results.first().row();
    }

    return -1;
}

QList<KPackage::Package> KCMLookandFeel::availablePackages(const QStringList &components)
{
    QList<KPackage::Package> packages;
    QStringList paths;
    const QStringList dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    paths.reserve(dataPaths.count());
    for (const QString &path : dataPaths) {
        QDir dir(path + QStringLiteral("/plasma/look-and-feel"));
        paths << dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }

    for (const QString &path : paths) {
        KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
        pkg.setPath(path);
        pkg.setFallbackPackage(KPackage::Package());
        if (components.isEmpty()) {
            packages << pkg;
        } else {
            for (const auto &component : components) {
                if (!pkg.filePath(component.toUtf8()).isEmpty()) {
                    packages << pkg;
                    break;
                }
            }
        }
    }

    return packages;
}

LookAndFeelSettings *KCMLookandFeel::lookAndFeelSettings() const
{
    return m_data->settings();
}

void KCMLookandFeel::loadModel()
{
    m_model->clear();

    const QList<KPackage::Package> pkgs = availablePackages({"defaults", "layouts"});
    for (const KPackage::Package &pkg : pkgs) {
        if (!pkg.metadata().isValid()) {
            continue;
        }
        QStandardItem *row = new QStandardItem(pkg.metadata().name());
        row->setData(pkg.metadata().pluginId(), PluginNameRole);
        row->setData(pkg.metadata().description(), DescriptionRole);
        row->setData(pkg.filePath("preview"), ScreenshotRole);
        row->setData(pkg.filePath("fullscreenpreview"), FullScreenPreviewRole);

        //What the package provides
        row->setData(!pkg.filePath("splashmainscript").isEmpty(), HasSplashRole);
        row->setData(!pkg.filePath("lockscreenmainscript").isEmpty(), HasLockScreenRole);
        row->setData(!pkg.filePath("runcommandmainscript").isEmpty(), HasRunCommandRole);
        row->setData(!pkg.filePath("logoutmainscript").isEmpty(), HasLogoutRole);

        if (!pkg.filePath("defaults").isEmpty()) {
            KSharedConfigPtr conf = KSharedConfig::openConfig(pkg.filePath("defaults"));
            KConfigGroup cg(conf, "kdeglobals");
            cg = KConfigGroup(&cg, "General");
            bool hasColors = !cg.readEntry("ColorScheme", QString()).isEmpty();
            if (!hasColors) {
                hasColors = !pkg.filePath("colors").isEmpty();
            }
            row->setData(hasColors, HasColorsRole);
            cg = KConfigGroup(&cg, "KDE");
            row->setData(!cg.readEntry("widgetStyle", QString()).isEmpty(), HasWidgetStyleRole);
            cg = KConfigGroup(conf, "kdeglobals");
            cg = KConfigGroup(&cg, "Icons");
            row->setData(!cg.readEntry("Theme", QString()).isEmpty(), HasIconsRole);

            cg = KConfigGroup(conf, "kdeglobals");
            cg = KConfigGroup(&cg, "Theme");
            row->setData(!cg.readEntry("name", QString()).isEmpty(), HasPlasmaThemeRole);

            cg = KConfigGroup(conf, "kcminputrc");
            cg = KConfigGroup(&cg, "Mouse");
            row->setData(!cg.readEntry("cursorTheme", QString()).isEmpty(), HasCursorsRole);

            cg = KConfigGroup(conf, "kwinrc");
            cg = KConfigGroup(&cg, "WindowSwitcher");
            row->setData(!cg.readEntry("LayoutName", QString()).isEmpty(), HasWindowSwitcherRole);

            cg = KConfigGroup(conf, "kwinrc");
            cg = KConfigGroup(&cg, "DesktopSwitcher");
            row->setData(!cg.readEntry("LayoutName", QString()).isEmpty(), HasDesktopSwitcherRole);
        }

        m_model->appendRow(row);
    }
    m_model->sort(0 /*column*/);

    //Model has been cleared so pretend the selected look and fell changed to force view update
    emit lookAndFeelSettings()->lookAndFeelPackageChanged();
}

void KCMLookandFeel::load()
{
    ManagedConfigModule::load();

    m_package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
    m_package.setPath(lookAndFeelSettings()->lookAndFeelPackage());
}

void KCMLookandFeel::save()
{
    KPackage::Package package = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"));
    package.setPath(lookAndFeelSettings()->lookAndFeelPackage());

    if (!package.isValid()) {
        return;
    }

    ManagedConfigModule::save();

    if (m_resetDefaultLayout) {
        QDBusMessage message = QDBusMessage::createMethodCall(QStringLiteral("org.kde.plasmashell"), QStringLiteral("/PlasmaShell"),
                                                   QStringLiteral("org.kde.PlasmaShell"), QStringLiteral("loadLookAndFeelDefaultLayout"));

        QList<QVariant> args;
        args << lookAndFeelSettings()->lookAndFeelPackage();
        message.setArguments(args);

        QDBusConnection::sessionBus().call(message, QDBus::NoBlock);
    }

    if (!package.filePath("defaults").isEmpty()) {
        KSharedConfigPtr conf = KSharedConfig::openConfig(package.filePath("defaults"));
        KConfigGroup cg(conf, "kdeglobals");
        cg = KConfigGroup(&cg, "KDE");
        if (m_applyWidgetStyle) {
            QString widgetStyle = cg.readEntry("widgetStyle", QString());
            // Some global themes refer to breeze's widgetStyle with a lowercase b.
            if (widgetStyle == QStringLiteral("breeze")) {
                widgetStyle = QStringLiteral("Breeze");
            }
            setWidgetStyle(widgetStyle);
        }

        if (m_applyColors) {
            QString colorsFile = package.filePath("colors");
            KConfigGroup cg(conf, "kdeglobals");
            cg = KConfigGroup(&cg, "General");
            QString colorScheme = cg.readEntry("ColorScheme", QString());

            if (!colorsFile.isEmpty()) {
                if (!colorScheme.isEmpty()) {
                    setColors(colorScheme, colorsFile);
                } else {
                    setColors(package.metadata().name(), colorsFile);
                }
            } else if (!colorScheme.isEmpty()) {
                colorScheme.remove(QLatin1Char('\'')); // So Foo's does not become FooS
                QRegExp fixer(QStringLiteral("[\\W,.-]+(.?)"));
                int offset;
                while ((offset = fixer.indexIn(colorScheme)) >= 0) {
                    colorScheme.replace(offset, fixer.matchedLength(), fixer.cap(1).toUpper());
                }
                colorScheme.replace(0, 1, colorScheme.at(0).toUpper());

                //NOTE: why this loop trough all the scheme files?
                //the scheme theme name is an heuristic, there is no plugin metadata whatsoever.
                //is based on the file name stripped from weird characters or the
                //eventual id- prefix store.kde.org puts, so we can just find a
                //theme that ends as the specified name
                bool schemeFound = false;
                const QStringList schemeDirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("color-schemes"), QStandardPaths::LocateDirectory);
                for (const QString &dir : schemeDirs) {
                    const QStringList fileNames = QDir(dir).entryList(QStringList()<<QStringLiteral("*.colors"));
                    for (const QString &file : fileNames) {
                        if (file.endsWith(colorScheme + QStringLiteral(".colors"))) {
                            setColors(colorScheme, dir + QLatin1Char('/') + file);
                            schemeFound = true;
                            break;
                        }
                    }
                    if (schemeFound) {
                        break;
                    }
                }
            }
        }

        if (m_applyIcons) {
            cg = KConfigGroup(conf, "kdeglobals");
            cg = KConfigGroup(&cg, "Icons");
            setIcons(cg.readEntry("Theme", QString()));
        }

        if (m_applyPlasmaTheme) {
            cg = KConfigGroup(conf, "plasmarc");
            cg = KConfigGroup(&cg, "Theme");
            setPlasmaTheme(cg.readEntry("name", QString()));
        }

        if (m_applyCursors) {
            cg = KConfigGroup(conf, "kcminputrc");
            cg = KConfigGroup(&cg, "Mouse");
            setCursorTheme(cg.readEntry("cursorTheme", QString()));
        }

        if (m_applyWindowSwitcher) {
            cg = KConfigGroup(conf, "kwinrc");
            cg = KConfigGroup(&cg, "WindowSwitcher");
            setWindowSwitcher(cg.readEntry("LayoutName", QString()));
        }

        if (m_applyDesktopSwitcher) {
            cg = KConfigGroup(conf, "kwinrc");
            cg = KConfigGroup(&cg, "DesktopSwitcher");
            setDesktopSwitcher(cg.readEntry("LayoutName", QString()));
        }

        if (m_applyWindowDecoration) {
            cg = KConfigGroup(conf, "kwinrc");
            cg = KConfigGroup(&cg, "org.kde.kdecoration2");
#ifdef HAVE_BREEZE_DECO
            setWindowDecoration(cg.readEntry("library", QStringLiteral(BREEZE_KDECORATION_PLUGIN_ID)), cg.readEntry("theme", QStringLiteral("Breeze")));
#else
            setWindowDecoration(cg.readEntry("library", QStringLiteral("org.kde.kwin.aurorae")), cg.readEntry("theme", QStringLiteral("kwin4_decoration_qml_plastik")));
#endif
        }

        // Reload KWin if something changed, but only once.
        if (m_applyWindowSwitcher || m_applyDesktopSwitcher || m_applyWindowDecoration) {
            QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KWin"),
                                                    QStringLiteral("org.kde.KWin"),
                                                    QStringLiteral("reloadConfig"));
            QDBusConnection::sessionBus().send(message);
        }

        //autostart
        if (m_resetDefaultLayout) {
            //remove all the old package to autostart
            {
                KSharedConfigPtr oldConf = KSharedConfig::openConfig(m_package.filePath("defaults"));
                cg = KConfigGroup(oldConf, QStringLiteral("Autostart"));
                const QStringList autostartServices = cg.readEntry("Services", QStringList());

                if (qEnvironmentVariableIsSet("KDE_FULL_SESSION")) {
                    for (const QString &serviceFile : autostartServices) {
                        KService service(serviceFile + QStringLiteral(".desktop"));
                        KAutostart as(serviceFile);
                        as.setAutostarts(false);
                        //FIXME: quite ugly way to stop things, and what about non KDE things?
                        QProcess::startDetached(QStringLiteral("kquitapp5"), {QStringLiteral("--service"), service.property(QStringLiteral("X-DBUS-ServiceName")).toString()});
                    }
                }
            }
            //Set all the stuff in the new lnf to autostart
            {
                cg = KConfigGroup(conf, QStringLiteral("Autostart"));
                const QStringList autostartServices = cg.readEntry("Services", QStringList());

                for (const QString &serviceFile : autostartServices) {
                    KService::Ptr service{new KService(serviceFile + QStringLiteral(".desktop"))};
                    KAutostart as(serviceFile);
                    as.setCommand(service->exec());
                    as.setAutostarts(true);
                    if (qEnvironmentVariableIsSet("KDE_FULL_SESSION")) {
                        auto *job = new KIO::ApplicationLauncherJob(service);
                        job->setUiDelegate(new KDialogJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
                        job->start();
                    }
                }
            }
        }
    }

    //TODO: option to enable/disable apply? they don't seem required by UI design
    const auto *item = m_model->item(pluginIndex(lookAndFeelSettings()->lookAndFeelPackage()));
    if (item->data(HasSplashRole).toBool()) {
        setSplashScreen(lookAndFeelSettings()->lookAndFeelPackage());
    }
    setLockScreen(lookAndFeelSettings()->lookAndFeelPackage());

    m_configGroup.sync();
    m_package.setPath(lookAndFeelSettings()->lookAndFeelPackage());
    runRdb(KRdbExportQtColors | KRdbExportGtkTheme | KRdbExportColors | KRdbExportQtSettings | KRdbExportXftSettings);
}

void KCMLookandFeel::setWidgetStyle(const QString &style)
{
    if (style.isEmpty()) {
        return;
    }

    // Some global themes use styles that may not be installed.
    // Test if style can be installed before updating the config.
    QScopedPointer<QStyle> newStyle(QStyleFactory::create(style));
    if (newStyle) {
        m_configGroup.writeEntry("widgetStyle", style, KConfig::Notify);
        m_configGroup.sync();
        //FIXME: changing style on the fly breaks QQuickWidgets
        KGlobalSettings::self()->emitChange(KGlobalSettings::StyleChanged);
    }
}

void KCMLookandFeel::setColors(const QString &scheme, const QString &colorFile)
{
    if (scheme.isEmpty() && colorFile.isEmpty()) {
        return;
    }

    KSharedConfigPtr conf = KSharedConfig::openConfig(colorFile, KSharedConfig::CascadeConfig);
    foreach (const QString &grp, conf->groupList()) {
        KConfigGroup cg(conf, grp);
        KConfigGroup cg2(&m_config, grp);
        cg.copyTo(&cg2, KConfig::Notify);
    }

    KConfigGroup configGroup(&m_config, "General");
    configGroup.writeEntry("ColorScheme", scheme, KConfig::Notify);

    configGroup.sync();
    KGlobalSettings::self()->emitChange(KGlobalSettings::PaletteChanged);
}

void KCMLookandFeel::setIcons(const QString &theme)
{
    if (theme.isEmpty()) {
        return;
    }

    KConfigGroup cg(&m_config, "Icons");
    cg.writeEntry("Theme", theme, KConfig::Notify);
    cg.sync();

    for (int i=0; i < KIconLoader::LastGroup; i++) {
        KIconLoader::emitChange(KIconLoader::Group(i));
    }
}

void KCMLookandFeel::setPlasmaTheme(const QString &theme)
{
    if (theme.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("plasmarc"));
    KConfigGroup cg(&config, "Theme");
    cg.writeEntry("name", theme);
    cg.sync();
}

void KCMLookandFeel::setCursorTheme(const QString themeName)
{
    //TODO: use pieces of cursor kcm when moved to plasma-desktop
    if (themeName.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("kcminputrc"));
    KConfigGroup cg(&config, "Mouse");
    cg.writeEntry("cursorTheme", themeName, KConfig::Notify);
    cg.sync();

#ifdef HAVE_XCURSOR
    // Require the Xcursor version that shipped with X11R6.9 or greater, since
    // in previous versions the Xfixes code wasn't enabled due to a bug in the
    // build system (freedesktop bug #975).
#if defined(HAVE_XFIXES) && XFIXES_MAJOR >= 2 && XCURSOR_LIB_VERSION >= 10105
    const int cursorSize = cg.readEntry("cursorSize", 24);

    QDir themeDir = cursorThemeDir(themeName, 0);

    if (!themeDir.exists()) {
        return;
    }

    XCursorTheme theme(themeDir);

    if (!CursorTheme::haveXfixes()) {
        return;
    }

    UpdateLaunchEnvJob launchEnvJob(QStringLiteral("XCURSOR_THEME"), themeName);

    // Update the Xcursor X resources
    runRdb(0);

    // Notify all applications that the cursor theme has changed
    KGlobalSettings::self()->emitChange(KGlobalSettings::CursorChanged);

    // Reload the standard cursors
    QStringList names;

    // Qt cursors
    names << QStringLiteral("left_ptr")       << QStringLiteral("up_arrow")      << QStringLiteral("cross")      << QStringLiteral("wait")
          << QStringLiteral("left_ptr_watch") << QStringLiteral("ibeam")         << QStringLiteral("size_ver")   << QStringLiteral("size_hor")
          << QStringLiteral("size_bdiag")     << QStringLiteral("size_fdiag")    << QStringLiteral("size_all")   << QStringLiteral("split_v")
          << QStringLiteral("split_h")        << QStringLiteral("pointing_hand") << QStringLiteral("openhand")
          << QStringLiteral("closedhand")     << QStringLiteral("forbidden")     << QStringLiteral("whats_this") << QStringLiteral("copy") << QStringLiteral("move") << QStringLiteral("link");

    // X core cursors
    names << QStringLiteral("X_cursor")            << QStringLiteral("right_ptr")           << QStringLiteral("hand1")
          << QStringLiteral("hand2")               << QStringLiteral("watch")               << QStringLiteral("xterm")
          << QStringLiteral("crosshair")           << QStringLiteral("left_ptr_watch")      << QStringLiteral("center_ptr")
          << QStringLiteral("sb_h_double_arrow")   << QStringLiteral("sb_v_double_arrow")   << QStringLiteral("fleur")
          << QStringLiteral("top_left_corner")     << QStringLiteral("top_side")            << QStringLiteral("top_right_corner")
          << QStringLiteral("right_side")          << QStringLiteral("bottom_right_corner") << QStringLiteral("bottom_side")
          << QStringLiteral("bottom_left_corner")  << QStringLiteral("left_side")           << QStringLiteral("question_arrow")
          << QStringLiteral("pirate");

    foreach (const QString &name, names) {
        XFixesChangeCursorByName(QX11Info::display(), theme.loadCursor(name, cursorSize), QFile::encodeName(name));
    }

#else
    KMessageBox::information(this,
                                 i18n("You have to restart the Plasma session for these changes to take effect."),
                                 i18n("Cursor Settings Changed"), "CursorSettingsChanged");
#endif
#endif
}

QDir KCMLookandFeel::cursorThemeDir(const QString &theme, const int depth)
{
    // Prevent infinite recursion
    if (depth > 10) {
        return QDir();
    }

    // Search each icon theme directory for 'theme'
    foreach (const QString &baseDir, cursorSearchPaths()) {
        QDir dir(baseDir);
        if (!dir.exists() || !dir.cd(theme)) {
            continue;
        }

        // If there's a cursors subdir, we'll assume this is a cursor theme
        if (dir.exists(QStringLiteral("cursors"))) {
            return dir;
        }

        // If the theme doesn't have an index.theme file, it can't inherit any themes.
        if (!dir.exists(QStringLiteral("index.theme"))) {
            continue;
        }

        // Open the index.theme file, so we can get the list of inherited themes
        KConfig config(dir.path() + QStringLiteral("/index.theme"), KConfig::NoGlobals);
        KConfigGroup cg(&config, "Icon Theme");

        // Recurse through the list of inherited themes, to check if one of them
        // is a cursor theme.
        QStringList inherits = cg.readEntry("Inherits", QStringList());
        foreach (const QString &inherit, inherits) {
            // Avoid possible DoS
            if (inherit == theme) {
                continue;
            }

            if (cursorThemeDir(inherit, depth + 1).exists()) {
                return dir;
            }
        }
    }

    return QDir();
}

const QStringList KCMLookandFeel::cursorSearchPaths()
{
    if (!m_cursorSearchPaths.isEmpty())
        return m_cursorSearchPaths;

#ifdef HAVE_XCURSOR
#if XCURSOR_LIB_MAJOR == 1 && XCURSOR_LIB_MINOR < 1
    // These are the default paths Xcursor will scan for cursor themes
    QString path("~/.icons:/usr/share/icons:/usr/share/pixmaps:/usr/X11R6/lib/X11/icons");

    // If XCURSOR_PATH is set, use that instead of the default path
    char *xcursorPath = std::getenv("XCURSOR_PATH");
    if (xcursorPath)
        path = xcursorPath;
#else
    // Get the search path from Xcursor
    QString path = XcursorLibraryPath();
#endif

    // Separate the paths
    m_cursorSearchPaths = path.split(QLatin1Char(':'), Qt::SkipEmptyParts);

    // Remove duplicates
    QMutableStringListIterator i(m_cursorSearchPaths);
    while (i.hasNext())
    {
        const QString path = i.next();
        QMutableStringListIterator j(i);
        while (j.hasNext())
            if (j.next() == path)
                j.remove();
    }

    // Expand all occurrences of ~/ to the home dir
    m_cursorSearchPaths.replaceInStrings(QRegExp(QStringLiteral("^~\\/")), QDir::home().path() + QLatin1Char('/'));
    return m_cursorSearchPaths;
#else
    return QStringList();
#endif
}

void KCMLookandFeel::setSplashScreen(const QString &theme)
{
    if (theme.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("ksplashrc"));
    KConfigGroup cg(&config, "KSplash");
    cg.writeEntry("Theme", theme);
    //TODO: a way to set none as spash in the l&f
    cg.writeEntry("Engine", "KSplashQML");
    cg.sync();
}

void KCMLookandFeel::setLockScreen(const QString &theme)
{
    if (theme.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("kscreenlockerrc"));
    KConfigGroup cg(&config, "Greeter");
    cg.writeEntry("Theme", theme);
    cg.sync();
}

void KCMLookandFeel::setWindowSwitcher(const QString &theme)
{
    if (theme.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("kwinrc"));
    KConfigGroup cg(&config, "TabBox");
    cg.writeEntry("LayoutName", theme);
    cg.sync();
}

void KCMLookandFeel::setDesktopSwitcher(const QString &theme)
{
    if (theme.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("kwinrc"));
    KConfigGroup cg(&config, "TabBox");
    cg.writeEntry("DesktopLayout", theme);
    cg.writeEntry("DesktopListLayout", theme);
    cg.sync();
}

void KCMLookandFeel::setWindowDecoration(const QString &library, const QString &theme)
{
    if (library.isEmpty()) {
        return;
    }

    KConfig config(QStringLiteral("kwinrc"));
    KConfigGroup cg(&config, "org.kde.kdecoration2");
    cg.writeEntry("library", library);
    cg.writeEntry("theme", theme, KConfig::Notify);

    cg.sync();
    // Reload KWin.
    QDBusMessage message = QDBusMessage::createSignal(QStringLiteral("/KWin"),
                                                      QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("reloadConfig"));
    QDBusConnection::sessionBus().send(message);
}

void KCMLookandFeel::setResetDefaultLayout(bool reset)
{
    if (m_resetDefaultLayout == reset) {
        return;
    }
    m_resetDefaultLayout = reset;
    emit resetDefaultLayoutChanged();
    if (reset) {
        setNeedsSave(true);
    }
}

bool KCMLookandFeel::resetDefaultLayout() const
{
    return m_resetDefaultLayout;
}
