/*
 * Copyright © 2003-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef THEMEPAGE_H
#define THEMEPAGE_H

#include <QWidget>
#include <QModelIndex>

#include "ui_themepage.h"

class CursorThemeModel;
class SortProxyModel;
class CursorTheme;

class ThemePage : public QWidget, private Ui::ThemePage
{
    Q_OBJECT

    public:
        ThemePage(QWidget* parent = 0);
        ~ThemePage();

        // Called by the KCM
        void save();
        void load();
        void defaults();

    Q_SIGNALS:
        void changed(bool);

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
        void getNewClicked();
        void installClicked();
        void removeClicked();

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

        CursorThemeModel *model;
        SortProxyModel *proxy;

        int appliedSize;
        // This index refers to the CursorThemeModel, not the proxy or the view
        QPersistentModelIndex appliedIndex;
};

#endif // THEMEPAGE_H
