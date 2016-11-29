/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>

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

#ifndef _KCM_DESKTOPTHEME_H
#define _KCM_DESKTOPTHEME_H

#include <KQuickAddons/ConfigModule>

namespace Plasma {
    class Svg;
    class Theme;
}

class QStandardItemModel;

class KCMDesktopTheme : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QStandardItemModel *desktopThemeModel READ desktopThemeModel CONSTANT)
    Q_PROPERTY(QString selectedPlugin READ selectedPlugin WRITE setSelectedPlugin NOTIFY selectedPluginChanged)

public:
    enum Roles {
        PluginNameRole = Qt::UserRole + 1,
        ThemeNameRole,
        IsLocalRole
    };
    Q_ENUM(Roles)

    KCMDesktopTheme(QObject *parent, const QVariantList &args);
    ~KCMDesktopTheme();

    QStandardItemModel *desktopThemeModel() const;

    QString selectedPlugin() const;
    void setSelectedPlugin(const QString &plugin);

    Q_INVOKABLE void getNewThemes();
    Q_INVOKABLE void installThemeFromFile(const QUrl &file);
    Q_INVOKABLE void removeTheme(const QString &name);

    Q_INVOKABLE void applyPlasmaTheme(QQuickItem *item, const QString &themeName);

    Q_INVOKABLE int indexOf(const QString &themeName) const;

Q_SIGNALS:
    void selectedPluginChanged(const QString &plugin);
    void showInfoMessage(const QString &infoMessage);

public Q_SLOTS:
    void load();
    void save();
    void defaults();

private:
    void removeThemes();
    void updateNeedsSave();

    QStandardItemModel *m_model;
    QString m_selectedPlugin;
    QStringList m_pendingRemoval;
    Plasma::Theme *m_defaultTheme;
    QHash<QString, Plasma::Theme*> m_themes;
};

Q_DECLARE_LOGGING_CATEGORY(KCM_DESKTOP_THEME)

#endif // _KCM_DESKTOPTHEME_H
