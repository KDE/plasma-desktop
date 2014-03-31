/***************************************************************************
 *   Copyright (C) 2003 by Martin Koller                                   *
 *   kollix@aon.at                                                         *
 *   This file is part of the KDE Control Center Module for Joysticks      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

#define KDE3_SUPPORT
#include <kcmodule.h>
#undef KDE3_SUPPORT

class JoyWidget;


/* on FreeBSD the header <sys/joystick.h> already has a struct joystick, so we can't use the same name here, Alex */
class Joystick: public KCModule
{
  Q_OBJECT

  public:
    explicit Joystick(QWidget *parent = 0, const QVariantList &list = QVariantList());

    virtual void load();
    virtual void defaults();

  private:
    JoyWidget *joyWidget;
};

#endif
