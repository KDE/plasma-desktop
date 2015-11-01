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

#include <QApplication>
#include <QStyle>
#include <QCursor>
#include <QImage>
#include <QFile>
#include <QX11Info>

#include "cursortheme.h"

#include <config-X11.h>

#ifdef HAVE_XFIXES
#  include <X11/Xlib.h>
#  include <X11/extensions/Xfixes.h>
#endif


CursorTheme::CursorTheme(const QString &title, const QString &description)
{
    setTitle(title);
    setDescription(description);
    setSample(QStringLiteral("left_ptr"));
    setIsHidden(false);
    setIsWritable(false);
}


QPixmap CursorTheme::icon() const
{
    if (m_icon.isNull())
        m_icon = createIcon();

    return m_icon;
}


QImage CursorTheme::autoCropImage(const QImage &image) const
{
    // Compute an autocrop rectangle for the image
    QRect r(image.rect().bottomRight(), image.rect().topLeft());
    const quint32 *pixels = reinterpret_cast<const quint32*>(image.bits());

    for (int y = 0; y < image.height(); y++)
    {
        for (int x = 0; x < image.width(); x++)
        {
            if (*(pixels++))
            {
                if (x < r.left())   r.setLeft(x);
                if (x > r.right())  r.setRight(x);
                if (y < r.top())    r.setTop(y);
                if (y > r.bottom()) r.setBottom(y);
            }
        }
    }

    // Normalize the rectangle
    return image.copy(r.normalized());
}


QPixmap CursorTheme::loadPixmap(const QString &name, int size) const
{
    QImage image = loadImage(name, size);
    if (image.isNull())
        return QPixmap();

    return QPixmap::fromImage(image);
}


static int nominalCursorSize(int iconSize)
{
    for (int i = 512; i > 8; i /= 2)
    {
        if (i < iconSize)
            return i;

        if ((i * .75) < iconSize)
            return int(i * .75);
    }

    return 8;
}


QPixmap CursorTheme::createIcon() const
{
    int iconSize = QApplication::style()->pixelMetric(QStyle::PM_LargeIconSize);
    int cursorSize = nominalCursorSize(iconSize);
    QSize size = QSize(iconSize, iconSize);

    QPixmap pixmap = createIcon(cursorSize);

    if (!pixmap.isNull())
    {
        // Scale the pixmap if it's larger than the preferred icon size
        if (pixmap.width() > size.width() || pixmap.height() > size.height())
            pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    return pixmap;
}


QPixmap CursorTheme::createIcon(int size) const
{
    QPixmap pixmap;
    QImage image = loadImage(sample(), size);

    if (image.isNull() && sample() != QLatin1String("left_ptr"))
        image = loadImage(QStringLiteral("left_ptr"), size);

    if (!image.isNull())
    {
        pixmap = QPixmap::fromImage(image);
    }

    return pixmap;
}


void CursorTheme::setCursorName(qulonglong cursor, const QString &name) const
{
#ifdef HAVE_XFIXES

    if (haveXfixes())
    {
        XFixesSetCursorName(QX11Info::display(), cursor,
                            QFile::encodeName(name));
    }
#endif
}

bool CursorTheme::haveXfixes()
{
    bool result = false;

#ifdef HAVE_XFIXES
    if (!QX11Info::isPlatformX11()) {
        return result;
    }
    int event_base, error_base;
    if (XFixesQueryExtension(QX11Info::display(), &event_base, &error_base))
    {
        int major, minor;
        XFixesQueryVersion(QX11Info::display(), &major, &minor);
        result = (major >= 2);
    }
#endif

    return result;
}
