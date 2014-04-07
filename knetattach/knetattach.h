/*
   Copyright (C) 2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */

#ifndef KNETATTACH_H
#define KNETATTACH_H

#include "ui_knetattach.h"

class KUrl;
class KNetAttach : public QWizard, private Ui_KNetAttach
{
    Q_OBJECT

public:
    explicit KNetAttach( QWidget* parent = 0 );

public slots:
    virtual void setInformationText( const QString & type );

private:
    QString _type;

    bool doConnectionTest( const KUrl & url );
    bool updateForProtocol( const QString & protocol );

private slots:
    void updateParametersPageStatus();
    bool validateCurrentPage();
    void updatePort( bool encryption );
    void updateFinishButtonText( bool save );
    void slotHelpClicked();
    void slotPageChanged( int );
};

#endif
