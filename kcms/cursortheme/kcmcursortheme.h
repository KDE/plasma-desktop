/*
 *  Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *  Copyright © 2019 Benjamin Port <benjamin.port@enioka.com>
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

#ifndef KCMCURSORTHEME_H
#define KCMCURSORTHEME_H

#include <KQuickAddons/ManagedConfigModule>
#include <QScopedPointer>

class QQmlListReference;
class QStandardItemModel;
class QTemporaryFile;

class CursorThemeModel;
class SortProxyModel;
class CursorTheme;
class CursorThemeSettings;
class CursorThemeData;

namespace KIO
{
    class FileCopyJob;
}

class CursorThemeConfig : public KQuickAddons::ManagedConfigModule
{
    Q_OBJECT
    Q_PROPERTY(CursorThemeSettings *cursorThemeSettings READ cursorThemeSettings CONSTANT)
    Q_PROPERTY(bool canInstall READ canInstall WRITE setCanInstall NOTIFY canInstallChanged)
    Q_PROPERTY(bool canResize READ canResize WRITE setCanResize NOTIFY canResizeChanged)
    Q_PROPERTY(bool canConfigure READ canConfigure WRITE setCanConfigure NOTIFY canConfigureChanged)
    Q_PROPERTY(QAbstractItemModel *cursorsModel READ cursorsModel CONSTANT)
    Q_PROPERTY(QAbstractItemModel *sizesModel READ sizesModel CONSTANT)

    Q_PROPERTY(bool downloadingFile READ downloadingFile NOTIFY downloadingFileChanged)
    Q_PROPERTY(int preferredSize READ preferredSize WRITE setPreferredSize NOTIFY preferredSizeChanged)

public:
    CursorThemeConfig(QObject *parent, const QVariantList &);
    ~CursorThemeConfig() override;

    void load() override;
    void save() override;
    void defaults() override;

    //for QML properties
    CursorThemeSettings *cursorThemeSettings() const;

    bool canInstall() const;
    void setCanInstall(bool can);

    bool canResize() const;
    void setCanResize(bool can);

    bool canConfigure() const;
    void setCanConfigure(bool can);

    int preferredSize() const;
    void setPreferredSize(int size);

    bool downloadingFile() const;

    QAbstractItemModel *cursorsModel();
    QAbstractItemModel *sizesModel();

    Q_INVOKABLE int cursorSizeIndex(int cursorSize) const;
    Q_INVOKABLE int cursorSizeFromIndex(int index);
    Q_INVOKABLE int cursorThemeIndex(const QString &cursorTheme) const;
    Q_INVOKABLE QString cursorThemeFromIndex(int index) const;

Q_SIGNALS:
    void canInstallChanged();
    void canResizeChanged();
    void canConfigureChanged();
    void downloadingFileChanged();
    void preferredSizeChanged();
    void themeApplied();

    void showSuccessMessage(const QString &message);
    void showInfoMessage(const QString &message);
    void showErrorMessage(const QString &message);

public Q_SLOTS:
    void ghnsEntriesChanged(const QQmlListReference &changedEntries);
    void installThemeFromFile(const QUrl &url);

private Q_SLOTS:
    /** Updates the size combo box. It loads the size list of the selected cursor
        theme with the corresponding icons and chooses an appropriate entry. It
        enables the combo box and the label if the theme provides more than one
        size, otherwise it disables it. If the size setting is looked in kiosk
        mode, it stays always disabled. */
    void updateSizeComboBox();


private:
    bool isSaveNeeded() const override;
    void installThemeFile(const QString &path);
    /** Applies a given theme, using XFixes, XCursor and KGlobalSettings.
        @param theme The cursor theme to be applied. It is save to pass 0 here
            (will result in \e false as return value).
        @param size The size hint that is used to select the cursor size.
        @returns If the changes could be applied. Will return \e false if \e theme is
            0 or if the XFixes and XCursor libraries aren't available in the required
            version, otherwise returns \e true. */
    bool applyTheme(const CursorTheme *theme, const int size);
    bool iconsIsWritable() const;
    void removeThemes();


    CursorThemeModel *m_themeModel;
    SortProxyModel *m_themeProxyModel;
    QStandardItemModel *m_sizesModel;
    CursorThemeData *m_data;

/** Holds the last size that was chosen by the user. Example: The user chooses
    theme1 which provides the sizes 24 and 36. He chooses 36. preferredSize gets
    set to 36. Now, he switches to theme2 which provides the sizes 30 and 40.
    preferredSize still is 36, so the UI will default to 40, which is next to 36.
    Now, he chooses theme3 which provides the sizes 34 and 44. preferredSize is
    still 36, so the UI defaults to 34. Now the user changes manually to 44. This
    will also change preferredSize. */
    int m_preferredSize;

    bool m_canInstall;
    bool m_canResize;
    bool m_canConfigure;

    QScopedPointer<QTemporaryFile> m_tempInstallFile;
    QPointer<KIO::FileCopyJob> m_tempCopyJob;
};

#endif
