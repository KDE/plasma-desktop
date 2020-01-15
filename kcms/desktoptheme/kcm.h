/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>
   Copyright (c) 2018 Kai Uwe Broulik <kde@privat.broulik.de>
   Copyright (c) 2019 Kevin Ottens <kevin.ottens@enioka.com>

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

#include <KQuickAddons/ManagedConfigModule>

class QTemporaryFile;

namespace Plasma {
    class Theme;
}

namespace KIO
{
class FileCopyJob;
}

class QQuickItem;
class DesktopThemeSettings;
class FilterProxyModel;
class ThemesModel;

class KCMDesktopTheme : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT

    Q_PROPERTY(DesktopThemeSettings *desktopThemeSettings READ desktopThemeSettings CONSTANT)
    Q_PROPERTY(FilterProxyModel *filteredModel READ filteredModel CONSTANT)
    Q_PROPERTY(ThemesModel *desktopThemeModel READ desktopThemeModel CONSTANT)
    Q_PROPERTY(bool downloadingFile READ downloadingFile NOTIFY downloadingFileChanged)
    Q_PROPERTY(bool canEditThemes READ canEditThemes CONSTANT)

public:

    KCMDesktopTheme(QObject *parent, const QVariantList &args);
    ~KCMDesktopTheme() override;

    DesktopThemeSettings *desktopThemeSettings() const;
    ThemesModel *desktopThemeModel() const;
    FilterProxyModel *filteredModel() const;

    bool downloadingFile() const;

    bool canEditThemes() const;

    Q_INVOKABLE void installThemeFromFile(const QUrl &url);

    Q_INVOKABLE void applyPlasmaTheme(QQuickItem *item, const QString &themeName);

    Q_INVOKABLE void editTheme(const QString &themeName);

Q_SIGNALS:
    void downloadingFileChanged();

    void showSuccessMessage(const QString &message);
    void showErrorMessage(const QString &message);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

private:
    bool isSaveNeeded() const override;

    void processPendingDeletions();

    void installTheme(const QString &path);

    DesktopThemeSettings *m_settings;

    ThemesModel *m_model;
    FilterProxyModel *m_filteredModel;
    QHash<QString, Plasma::Theme*> m_themes;
    bool m_haveThemeExplorerInstalled;

    QScopedPointer<QTemporaryFile> m_tempInstallFile;
    QPointer<KIO::FileCopyJob> m_tempCopyJob;
};

Q_DECLARE_LOGGING_CATEGORY(KCM_DESKTOP_THEME)

#endif // _KCM_DESKTOPTHEME_H
