/*
    Copyright (C) 2000,2002 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (C) 2005,2006 Olivier Goffart <ogoffart at kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/


#ifndef _KNOTIFY_H
#define _KNOTIFY_H


#include <kcmodule.h>

class KComboBox;
class QPushButton;
class KNotifyConfigWidget;

class KCMKNotify : public KCModule
{
    Q_OBJECT

public:
    KCMKNotify(QWidget *parent, const QVariantList &);
    virtual ~KCMKNotify();

    virtual void defaults();
    virtual void save();

public Q_SLOTS:
    virtual void load();

private Q_SLOTS:
    void slotAppActivated( const int &);

private:

    KComboBox *m_appCombo;
    KNotifyConfigWidget *m_notifyWidget;
    QPushButton *m_psb;

};

#endif
