/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>

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

#ifndef _KCM_SEARCH_H
#define _KCM_SEARCH_H

#include <KConfig>
#include <KConfigGroup>
#include <QListWidget>
#include <QDir>

#include <KNewStuff3/KNS3/DownloadDialog>

#include <Plasma/Package>
#include <KQuickAddons/ConfigModule>

class QQuickItem;
class QStandardItemModel;

class KCMLookandFeel : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QStandardItemModel *lookAndFeelModel READ lookAndFeelModel CONSTANT)
    Q_PROPERTY(QString selectedPlugin READ selectedPlugin WRITE setSelectedPlugin NOTIFY selectedPluginChanged)
    Q_PROPERTY(int selectedPluginIndex READ selectedPluginIndex NOTIFY selectedPluginIndexChanged)

    Q_PROPERTY(bool applyColors READ applyColors WRITE setApplyColors NOTIFY applyColorsChanged)
    Q_PROPERTY(bool applyWidgetStyle READ applyWidgetStyle WRITE setApplyWidgetStyle NOTIFY applyWidgetStyleChanged)
    Q_PROPERTY(bool applyIcons READ applyIcons WRITE setApplyIcons NOTIFY applyIconsChanged)
    Q_PROPERTY(bool applyPlasmaTheme READ applyPlasmaTheme WRITE setApplyPlasmaTheme NOTIFY applyPlasmaThemeChanged)
    Q_PROPERTY(bool applyWindowSwitcher READ applyWindowSwitcher WRITE setApplyWindowSwitcher NOTIFY applyWindowSwitcherChanged)
    Q_PROPERTY(bool applyDesktopSwitcher READ applyDesktopSwitcher WRITE setApplyDesktopSwitcher NOTIFY applyDesktopSwitcherChanged)
    Q_PROPERTY(bool resetDefaultLayout READ resetDefaultLayout WRITE setResetDefaultLayout NOTIFY resetDefaultLayoutChanged)

public:
    enum Roles {
        PluginNameRole = Qt::UserRole +1,
        ScreenhotRole,
        FullScreenPreviewRole,
        DescriptionRole,
        HasSplashRole,
        HasLockScreenRole,
        HasRunCommandRole,
        HasLogoutRole,
        HasColorsRole,
        HasWidgetStyleRole,
        HasIconsRole,
        HasPlasmaThemeRole,
        HasCursorsRole,
        HasWindowSwitcherRole,
        HasDesktopSwitcherRole
    };
    KCMLookandFeel(QObject* parent, const QVariantList& args);
    ~KCMLookandFeel() override;

    //List only packages which provide at least one of the components
    QList<Plasma::Package> availablePackages(const QStringList &components);

    QStandardItemModel *lookAndFeelModel();

    QString selectedPlugin() const;
    void setSelectedPlugin(const QString &plugin);

    int selectedPluginIndex() const;

    //Setters of the various theme pieces
    void setWidgetStyle(const QString &style);
    void setColors(const QString &scheme, const QString &colorFile);
    void setIcons(const QString &theme);
    void setPlasmaTheme(const QString &theme);
    void setCursorTheme(const QString theme);
    void setSplashScreen(const QString &theme);
    void setLockScreen(const QString &theme);
    void setWindowSwitcher(const QString &theme);
    void setDesktopSwitcher(const QString &theme);
    void setWindowDecoration(const QString &library, const QString &theme);

    void setApplyColors(bool apply);
    bool applyColors() const;
    void setApplyWidgetStyle(bool apply);
    bool applyWidgetStyle() const;
    void setApplyIcons(bool apply);
    bool applyIcons() const;
    void setApplyPlasmaTheme(bool apply);
    bool applyPlasmaTheme() const;
    void setApplyWindowSwitcher(bool apply);
    bool applyWindowSwitcher() const;
    void setApplyDesktopSwitcher(bool apply);
    bool applyDesktopSwitcher() const;
    bool resetDefaultLayout() const;
    void setResetDefaultLayout(bool reset);

    void loadModel();

    Q_INVOKABLE void getNewStuff(QQuickItem *ctx);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

Q_SIGNALS:
    void selectedPluginChanged();

    void applyColorsChanged();
    void applyWidgetStyleChanged();
    void applyIconsChanged();
    void applyPlasmaThemeChanged();
    void applyWindowSwitcherChanged();
    void applyDesktopSwitcherChanged();
    void resetDefaultLayoutChanged();
    void selectedPluginIndexChanged();

private:
    QDir cursorThemeDir(const QString &theme, const int depth);
    const QStringList cursorSearchPaths();
    QStandardItemModel *m_model;
    Plasma::Package m_package;
    QString m_selectedPlugin;
    QStringList m_cursorSearchPaths;
    QPointer<KNS3::DownloadDialog> m_newStuffDialog;

    KConfig m_config;
    KConfigGroup m_configGroup;

    bool m_applyColors : 1;
    bool m_applyWidgetStyle : 1;
    bool m_applyIcons : 1;
    bool m_applyPlasmaTheme : 1;
    bool m_applyCursors : 1;
    bool m_applyWindowSwitcher : 1;
    bool m_applyDesktopSwitcher : 1;
    bool m_resetDefaultLayout : 1;
    bool m_applyWindowDecoration : 1;
};

#endif
