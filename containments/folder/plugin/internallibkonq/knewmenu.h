/* This file is part of the KDE project
   Copyright (C) 1998-2009 David Faure <faure@kde.org>
                 2003      Sven Leiber <s.leiber@web.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 or at your option version 3.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KNEWMENU_H
#define KNEWMENU_H

#include <knewfilemenu.h>
#include "libkonq_export.h"

/**
 * @deprecated since 4.5, use KNewFileMenu from kdelibs/kfile instead.
 */
class LIBKONQ_EXPORT_DEPRECATED KNewMenu : public KNewFileMenu
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param collection the KActionCollection this KAction should be
     * added to.
     * @param parentWidget the parent widget that will be the owner of
     * this KNewMenu and that will take care of destroying this instance
     * once the parentWidget itself got destroyed.
     * @param name action name, when adding the action to the collection
     */
    KNewMenu(KActionCollection* collection, QWidget* parentWidget, const QString& name);

public Q_SLOTS:
    /**
     * Checks if updating the list is necessary
     * IMPORTANT : Call this in the slot for aboutToShow.
     * And while you're there, you probably want to call setViewShowsHiddenFiles ;)
     */
    void slotCheckUpToDate() {
        checkUpToDate();
    }

};

#endif
