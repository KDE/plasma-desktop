/***************************************************************************
                          smserverconfigimpl.cpp  -  description
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

#include "smserverconfigimpl.h"

SMServerConfigImpl::SMServerConfigImpl(QWidget *parent ) : SMServerConfigDlg(parent) {
    connect(confirmLogoutCheck,&QAbstractButton::toggled, this, &SMServerConfigImpl::configChanged);
    connect(previousSessionRadio,&QAbstractButton::toggled, this, &SMServerConfigImpl::configChanged);
    connect(savedSessionRadio,&QAbstractButton::toggled, this, &SMServerConfigImpl::configChanged);
    connect(emptySessionRadio,&QAbstractButton::toggled, this, &SMServerConfigImpl::configChanged);
    connect(logoutRadio,&QAbstractButton::toggled, this, &SMServerConfigImpl::configChanged);
    connect(haltRadio,&QAbstractButton::toggled, this, &SMServerConfigImpl::configChanged);
    connect(rebootRadio,&QAbstractButton::toggled, this, &SMServerConfigImpl::configChanged);
    connect(excludeLineedit,&QLineEdit::textChanged,this, &SMServerConfigImpl::configChanged);
    connect(offerShutdownCheck,&QAbstractButton::toggled,this, &SMServerConfigImpl::configChanged);

    firmwareSetupBox->hide();

    firmwareSetupMessageWidget->hide();
}
SMServerConfigImpl::~SMServerConfigImpl(){
}
/** No descriptions */
void SMServerConfigImpl::configChanged(){

 emit changed();

}
