/*
 * main.h
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * KDE Frameworks 5 port Copyright (C) 2013 Jonathan Riddell <jr@jriddell.org>
 * Copyright (c) 2018 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

#include <KQuickAddons/ConfigModule>

#include <KNS3/DownloadDialog>

#include <QScopedPointer>

class KIconTheme;

class QQuickItem;
class QTemporaryFile;

namespace KIO
{
class FileCopyJob;
}

class IconsModel;

class IconModule : public KQuickAddons::ConfigModule
{
    Q_OBJECT

    Q_PROPERTY(IconsModel *iconsModel READ iconsModel CONSTANT)

    Q_PROPERTY(QStringList iconGroups READ iconGroups CONSTANT)

    Q_PROPERTY(bool downloadingFile READ downloadingFile NOTIFY downloadingFileChanged)

public:
    IconModule(QObject *parent, const QVariantList &args);
    ~IconModule() override;

    enum Roles {
        ThemeNameRole = Qt::UserRole + 1,
        DescriptionRole,
        RemovableRole,
        PendingDeletionRole
    };

    IconsModel *iconsModel() const;

    QStringList iconGroups() const;

    bool downloadingFile() const;

    void load() override;
    void save() override;
    void defaults() override;

    Q_INVOKABLE void getNewStuff(QQuickItem *ctx);
    Q_INVOKABLE void installThemeFromFile(const QUrl &url);

    Q_INVOKABLE int iconSize(int group) const;
    Q_INVOKABLE void setIconSize(int group, int size);
    Q_INVOKABLE QList<int> availableIconSizes(int group) const;

    // QML doesn't understand QList<QPixmap>, hence wrapped in a QVariantList
    Q_INVOKABLE QVariantList previewIcons(const QString &themeName, int size, qreal dpr, int limit = -1);

signals:
    void iconSizesChanged();
    void downloadingFileChanged();

    void showSuccessMessage(const QString &message);
    void showErrorMessage(const QString &message);

    void showProgress(const QString &message);
    void hideProgress();

private:
    void loadIconSizes();

    void processPendingDeletions();

    static QStringList findThemeDirs(const QString &archiveName);
    bool installThemes(const QStringList &themes, const QString &archiveName);
    void installThemeFile(const QString &path);

    void exportToKDE4();

    static QPixmap getBestIcon(KIconTheme &theme, const QStringList &iconNames, int size, qreal dpr);

    IconsModel *m_model;
    // so we avoid launching changeicon process when theme didn't change (but only e.g. pending deletions)
    bool m_selectedThemeDirty = false;
    bool m_iconSizesDirty = false;

    // set when user hits "Defaults" button at which point we'll remove all custom icon effects on Apply
    bool m_revertIconEffects = false;

    QVector<int> m_iconSizes;

    QStringList m_iconGroups;

    QScopedPointer<QTemporaryFile> m_tempInstallFile;
    QPointer<KIO::FileCopyJob> m_tempCopyJob;

    QPointer<KNS3::DownloadDialog> m_newStuffDialog;

};
