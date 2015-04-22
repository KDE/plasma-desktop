/*
 *  Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
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

#include <KQuickAddons/ConfigModule>

class CursorThemeModel;
class SortProxyModel;
class CursorTheme;

class CursorThemeConfig : public KQuickAddons::ConfigModule
{
    Q_OBJECT
    Q_PROPERTY(bool canInstall READ canInstall WRITE setCanInstall NOTIFY canInstallChanged)
    Q_PROPERTY(bool canRemove READ canRemove WRITE setCanRemove NOTIFY canRemoveChanged)
    Q_PROPERTY(bool canResize READ canResize WRITE setCanResize NOTIFY canResizeChanged)
    Q_PROPERTY(bool canConfigure READ canConfigure WRITE setCanConfigure NOTIFY canConfigureChanged)

public:
    CursorThemeConfig(QObject *parent, const QVariantList &);
    ~CursorThemeConfig();

public:
    void load();
    void save();
    void defaults();

    //for QML properties
    bool canInstall() const;
    void setCanInstall(bool can);

    bool canRemove() const;
    void setCanRemove(bool can);

    bool canResize() const;
    void setCanResize(bool can);

    bool canConfigure() const;
    void setCanConfigure(bool can);

Q_SIGNALS:
    void canInstallChanged();
    void canRemoveChanged();
    void canResizeChanged();
    void canConfigureChanged();

public Q_SLOTS:
    void getNewClicked();
    void installClicked();
    void removeClicked();

private Q_SLOTS:
    void selectionChanged();
    /** Updates the preview. If the size has changed, it also emits changed() */
    void sizeChanged();
    /** Sets #preferredSize to the item that is currently selected in sizeComboBox.
        If none is selected, it is set to 0. */
    void preferredSizeChanged();
    /** Updates the size combo box. It loads the size list of the selected cursor
        theme with the corresponding icons and chooses an appropriate entry. It
        enables the combo box and the label if the theme provides more than one
        size, otherwise it disables it. If the size setting is looked in kiosk
        mode, it stays always disabled. */
    void updateSizeComboBox();


private:
    /** @returns 0 if in the UI "automatic size" is selected, otherwise
                    returns the custom size. */
    int selectedSize() const;
    /** Holds the last size that was choosen by the user. Example: The user chooses
        theme1 which provides the sizes 24 and 36. He chooses 36. preferredSize gets
        set to 36. Now, he switchs to theme2 which provides the sizes 30 and 40.
        preferredSize still is 36, so the UI will default to 40, which is next to 36.
        Now, he chooses theme3 which provides the sizes 34 and 44. preferredSize is
        still 36, so the UI defaults to 34. Now the user changes manually to 44. This
        will also change preferredSize. */
    int preferredSize;
    void updatePreview();
    QModelIndex selectedIndex() const;
    bool installThemes(const QString &file);
    /** Applies a given theme, using XFixes, XCursor and KGlobalSettings.
        @param theme The cursor theme to be applied. It is save to pass 0 here
            (will result in \e false as return value).
        @param size The size hint that is used to select the cursor size.
        @returns If the changes could be applied. Will return \e false if \e theme is
            0 or if the XFixes and XCursor libraries aren't available in the required
            version, otherwise returns \e true. */
    bool applyTheme(const CursorTheme *theme, const int size);
    bool iconsIsWritable() const;


    CursorThemeModel *m_model;
    SortProxyModel *m_proxyModel;

    int m_appliedSize;
    // This index refers to the CursorThemeModel, not the proxy or the view
    QPersistentModelIndex m_appliedIndex;

    bool m_canInstall;
    bool m_canRemove;
    bool m_canResize;
    bool m_canConfigure;
};

#endif
