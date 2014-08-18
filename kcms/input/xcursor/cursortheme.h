/*
 * Copyright © 2006-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 or at your option version 3 as published 
 * by the Free Software Foundation.
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

#ifndef CURSORTHEME_H
#define CURSORTHEME_H

#include <QPixmap>
#include <QHash>

/**
 * This is the abstract base class for all cursor themes stored in a
 * CursorThemeModel and previewed in a PreviewWidget.
 *
 * All cursor themes have a title, a description, an icon, and an internal
 * name, all of which, except for the internal name, CursorThemeModel
 * supplies to item views.
 *
 * A cursor theme may also have a path to the directory where the theme
 * is located in the filesystem. If isWritable() returns true, This directory
 * may be deleted in order to remove the theme at the users request.
 *
 * Subclasses must reimplement loadImage() and loadCursor(), which are
 * called by PreviewWidget to load cursors and cursor images. Subclasses may
 * optionally reimplement loadPixmap(), which in the default implementation
 * calls loadImage(), and converts the returned image to a pixmap.
 * Subclasses may also reimplement the protected function createIcon(),
 * which creates the icon pixmap that's supplied to item views. The default
 * implementation calls loadImage() to load the sample cursor, and creates
 * the icon from that.
 */
class CursorTheme
{
    public:
        enum ItemDataRole {
            // Note: use   printf "0x%08X\n" $(($RANDOM*$RANDOM))
            // to define additional roles.
            DisplayDetailRole = 0x24A3DAF8
        };

        CursorTheme() {}
        CursorTheme(const QString &title, const QString &description = QString());
        virtual ~CursorTheme() {}

        const QString title() const        { return m_title; }
        const QString description() const  { return m_description; }
        const QString sample() const       { return m_sample; }
        const QString name() const         { return m_name; }
        const QString path() const         { return m_path; }
        /** @returns A list of the available sizes in this cursor theme,
            @warning This list may be empty if the engine doesn't support
            the recognition of the size. */
        const QList<int> availableSizes() const
                                           { return m_availableSizes; }
        bool isWritable() const            { return m_writable; }
        bool isHidden() const              { return m_hidden; }
        QPixmap icon() const;

        /// Hash value for the internal name
        uint hash() const                  { return m_hash; }

        /// Loads the cursor image @p name, with the nominal size @p size.
        /// The image should be autocropped to the smallest possible size.
        /// If the theme doesn't have the cursor @p name, it should return a null image.
        virtual QImage loadImage(const QString &name, int size = 0) const = 0;

        /// Convenience function. Default implementation calls
        /// QPixmap::fromImage(loadImage());
        virtual QPixmap loadPixmap(const QString &name, int size = 0) const;

        /// Loads the cursor @p name, with the nominal size @p size.
        /// If the theme doesn't have the cursor @p name, it should return
        /// the default cursor from the active theme instead.
        virtual qulonglong loadCursor(const QString &name, int size = 0) const = 0;

        /** Creates the icon returned by @ref icon(). Don't use this function
            directly but use @ref icon() instead, because @ref icon() caches
            the icon.
            @returns A pixmap with a cursor (usually left_ptr) that can
            be used as icon for this theme. The size is adopted to
            standard icon sizes.*/
        virtual QPixmap createIcon() const;
        /** @returns A pixmap with a cursor (usually left_ptr) that can
            be used as icon for this theme. */
        virtual QPixmap createIcon(int size) const;

        static bool haveXfixes();

    protected:
        void setTitle( const QString &title )      { m_title       = title; }
        void setDescription( const QString &desc ) { m_description = desc; }
        void setSample( const QString &sample )    { m_sample      = sample; }
        inline void setName( const QString &name );
        void setPath( const QString &path )        { m_path        = path; }
        void setAvailableSizes( const QList<int> &availableSizes )
                                                   { m_availableSizes = availableSizes; }
        void setIcon( const QPixmap &icon )        { m_icon        = icon; }
        void setIsWritable( bool val )             { m_writable    = val; }
        void setIsHidden( bool val )               { m_hidden      = val; }

        /// Convenience function for cropping an image.
        QImage autoCropImage( const QImage &image ) const;

        // Convenience function that uses Xfixes to tag a cursor with a name
        void setCursorName(qulonglong cursor, const QString &name) const;

        QString m_title;
        QString m_description;
        QString m_path;
        QList<int> m_availableSizes;
        QString m_sample;
        mutable QPixmap m_icon;
        bool m_writable:1;
        bool m_hidden:1;

    private:
        QString m_name;
        uint m_hash;

        friend class CursorThemeModel;
};

void CursorTheme::setName(const QString &name)
{
    m_name = name;
    m_hash = qHash(name);
}

#endif // CURSORTHEME_H

