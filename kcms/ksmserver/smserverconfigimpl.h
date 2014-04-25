/***************************************************************************
                          smserverconfigimpl.h  -  description
                             -------------------
    begin                : Thu May 17 2001
    copyright            : (C) 2001 by stulle
    email                : stulle@tux
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SMSERVERCONFIGIMPL_H
#define SMSERVERCONFIGIMPL_H

#include <QWidget>
#include "ui_smserverconfigdlg.h"

/**
  *@author stulle
  */

class SMServerConfigDlg : public QWidget, public Ui::SMServerConfigDlg
{
public:
  SMServerConfigDlg( QWidget *parent ) : QWidget( parent ) {
    setupUi( this );
  }
};


class SMServerConfigImpl : public SMServerConfigDlg  {
   Q_OBJECT
public:
	SMServerConfigImpl(QWidget *parent=0);
	~SMServerConfigImpl();
public Q_SLOTS: // Public slots
  /** No descriptions */
  void configChanged();
Q_SIGNALS: // Signals
  /** No descriptions */
  void changed();
};

#endif
