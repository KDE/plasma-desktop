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

#include <KLocalizedString>
#include <KConfig>
#include <KConfigGroup>

#include <QCursor>
#include <QImage>
#include <QDir>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xcursor/Xcursor.h>

#include "xcursortheme.h"


// Static variable holding alternative names for some cursors
QHash<QString, QString> XCursorTheme::alternatives;

XCursorTheme::XCursorTheme(const QDir &themeDir)
    : CursorTheme(themeDir.dirName())
{
    // Directory information
    setName(themeDir.dirName());
    setPath(themeDir.path());
    setIsWritable(QFileInfo(themeDir.path()).isWritable()); // ### perhaps this shouldn't be cached

    if (themeDir.exists("index.theme"))
        parseIndexFile();

    QString cursorFile = path() + "/cursors/left_ptr";
    QList<int> sizeList;
    XcursorImages *images = XcursorFilenameLoadAllImages(qPrintable(cursorFile));
    if (images)
    {
        for (int i = 0; i < images->nimage; ++i)
        {
            if (!sizeList.contains(images->images[i]->size))
                sizeList.append(images->images[i]->size);
        };
        XcursorImagesDestroy(images);
        qSort(sizeList.begin(), sizeList.end());
        m_availableSizes = sizeList;
    };
    if (!sizeList.isEmpty())
    {
        QString sizeListString = QString::number(sizeList.takeFirst());
        while (!sizeList.isEmpty())
        {
            sizeListString.append(", ");
            sizeListString.append(QString::number(sizeList.takeFirst()));
        };
        QString tempString = i18nc(
            "@info The argument is the list of available sizes (in pixel). Example: "
                "'Available sizes: 24' or 'Available sizes: 24, 36, 48'",
            "(Available sizes: %1)",
            sizeListString);
        if (m_description.isEmpty())
          m_description = tempString;
        else
          m_description = m_description + ' ' + tempString;
    };
}


void XCursorTheme::parseIndexFile()
{
    KConfig config(path() + "/index.theme", KConfig::NoGlobals);
    KConfigGroup cg(&config, "Icon Theme");

    m_title       = cg.readEntry("Name",     m_title);
    m_description = cg.readEntry("Comment",  m_description);
    m_sample      = cg.readEntry("Example",  m_sample);
    m_hidden      = cg.readEntry("Hidden",   false);
    m_inherits    = cg.readEntry("Inherits", QStringList());
}


QString XCursorTheme::findAlternative(const QString &name) const
{
    if (alternatives.isEmpty())
    {
        alternatives.reserve(18);

        // Qt uses non-standard names for some core cursors.
        // If Xcursor fails to load the cursor, Qt creates it with the correct name using the
        // core protcol instead (which in turn calls Xcursor). We emulate that process here.
        // Note that there's a core cursor called cross, but it's not the one Qt expects.
        alternatives.insert("cross",          "crosshair");
        alternatives.insert("up_arrow",       "center_ptr");
        alternatives.insert("wait",           "watch");
        alternatives.insert("ibeam",          "xterm");
        alternatives.insert("size_all",       "fleur");
        alternatives.insert("pointing_hand",  "hand2");

        // Precomputed MD5 hashes for the hardcoded bitmap cursors in Qt and KDE.
        // Note that the MD5 hash for left_ptr_watch is for the KDE version of that cursor.
        alternatives.insert("size_ver",       "00008160000006810000408080010102");
        alternatives.insert("size_hor",       "028006030e0e7ebffc7f7070c0600140");
        alternatives.insert("size_bdiag",     "fcf1c3c7cd4491d801f1e1c78f100000");
        alternatives.insert("size_fdiag",     "c7088f0f3e6c8088236ef8e1e3e70000");
        alternatives.insert("whats_this",     "d9ce0ab605698f320427677b458ad60b");
        alternatives.insert("split_h",        "14fef782d02440884392942c11205230");
        alternatives.insert("split_v",        "2870a09082c103050810ffdffffe0204");
        alternatives.insert("forbidden",      "03b6e0fcb3499374a867c041f52298f0");
        alternatives.insert("left_ptr_watch", "3ecb610c1bf2410f44200f48c40d3599");
        alternatives.insert("hand2",          "e29285e634086352946a0e7090d73106");
        alternatives.insert("openhand",       "9141b49c8149039304290b508d208c40");
        alternatives.insert("closedhand",     "05e88622050804100c20044008402080");
    }

    return alternatives.value(name, QString());
}


XcursorImage *XCursorTheme::xcLoadImage(const QString &image, int size) const
{
    QByteArray cursorName = QFile::encodeName(image);
    QByteArray themeName  = QFile::encodeName(name());

    return XcursorLibraryLoadImage(cursorName, themeName, size);
}


XcursorImages *XCursorTheme::xcLoadImages(const QString &image, int size) const
{
    QByteArray cursorName = QFile::encodeName(image);
    QByteArray themeName  = QFile::encodeName(name());

    return XcursorLibraryLoadImages(cursorName, themeName, size);
}


int XCursorTheme::autodetectCursorSize() const
{
    /* This code is basically borrowed from display.c of the XCursor library
       We can't use "int XcursorGetDefaultSize(Display *dpy)" because if
       previously the cursor size was set to a custom value, it would return
       this custom value. */
    int size = 0;
    int dpi = 0;
    Display *dpy = QX11Info::display();
    // The string "v" is owned and will be destroyed by Xlib
    char *v = XGetDefault(dpy, "Xft", "dpi");
    if (v)
        dpi = atoi(v);
    if (dpi)
        size = dpi * 16 / 72;
    if (size == 0)
    {
        int dim;
        if (DisplayHeight(dpy, DefaultScreen(dpy)) <
            DisplayWidth(dpy, DefaultScreen(dpy)))
        {
            dim = DisplayHeight(dpy, DefaultScreen(dpy));
        } else {
            dim = DisplayWidth(dpy, DefaultScreen(dpy));
        };
        size = dim / 48;
    }
    return size;
}

qulonglong XCursorTheme::loadCursor(const QString &name, int size) const
{
    if (size <= 0)
        size = autodetectCursorSize();

    // Load the cursor images
    XcursorImages *images = xcLoadImages(name, size);

    if (!images)
        images = xcLoadImages(findAlternative(name), size);

    if (!images)
        return None;

    // Create the cursor
    Cursor handle = XcursorImagesLoadCursor(QX11Info::display(), images);
    XcursorImagesDestroy(images);

    setCursorName(handle, name);
    return handle;
}


QImage XCursorTheme::loadImage(const QString &name, int size) const
{
    if (size <= 0)
        size = autodetectCursorSize();

    // Load the image
    XcursorImage *xcimage = xcLoadImage(name, size);

    if (!xcimage)
        xcimage = xcLoadImage(findAlternative(name), size);

    if (!xcimage) {
        return QImage();
    }

    // Convert the XcursorImage to a QImage, and auto-crop it
    QImage image((uchar *)xcimage->pixels, xcimage->width, xcimage->height,
                 QImage::Format_ARGB32_Premultiplied );

    image = autoCropImage(image);
    XcursorImageDestroy(xcimage);

    return image;
}

