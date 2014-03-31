/**
 *  Copyright (c) Martin R. Jones 1996
 *  Copyright 1998-2007 David Faure <faure@kde.org>
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

// Summarized history:
//
// "Desktop Icons Options" Tab for KDesktop configuration
// Martin Jones
//
// Port to KControl, split from "Misc" Tab, Port to KControl2
// (c) David Faure 1998
// Desktop menus, paths
// (c) David Faure 2000

#ifndef __GLOBALPATHS_H
#define __GLOBALPATHS_H

#include <kcmodule.h>
#include <kio/global.h>
#include <kio/udsentry.h>
#include <QUrl>

class QFormLayout;
class KJob;
class KUrlRequester;
class QStringList;

namespace KIO { class Job; }

//-----------------------------------------------------------------------------
// The "Path" Tab contains :
// The paths for Desktop, Autostart and Documents

class DesktopPathConfig : public KCModule
{
    Q_OBJECT
public:
    DesktopPathConfig( QWidget *parent, const QVariantList &args );
    virtual void load();
    virtual void save();
    virtual void defaults();

private Q_SLOTS:
    void slotEntries( KIO::Job * job, const KIO::UDSEntryList& list);

private:
    KUrlRequester* addRow(QFormLayout *lay, const QString& label, const QString& whatsThis);
    bool xdgSavePath(KUrlRequester* ur, const QUrl& currentUrl, const char* xdgKey, const QString& type);

    // Desktop Paths
    KUrlRequester *urDesktop;
    KUrlRequester *urAutostart;
    KUrlRequester *urDocument;
    KUrlRequester *urDownload;
    KUrlRequester *urMovie;
    KUrlRequester *urPicture;
    KUrlRequester *urMusic;

    bool moveDir( const QUrl & src, const QUrl & dest, const QString & type );
    bool m_ok;
    QUrl m_copyToDest; // used when the destination directory already exists
    QUrl m_copyFromSrc;

private Q_SLOTS:
    void slotResult( KJob * job );
};

#endif

