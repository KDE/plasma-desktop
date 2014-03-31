/*
  Copyright (c) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)
                1998 Bernd Wuebben <wuebben@kde.org>
                2000 Matthias Elter <elter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef __bell_h__
#define __bell_h__

#include "kcmodule.h"

class QCheckBox;
class KIntNumInput;
class QPushButton;

class KBellConfig : public KCModule
{
  Q_OBJECT

 public:
  KBellConfig(QWidget *parent, const QVariantList &args);

  void load();
  void save();
  void defaults();

 protected Q_SLOTS:
  void ringBell();
  void useBell( bool );

 private:
  QPushButton  *m_testButton;
  KIntNumInput *m_volume;
  KIntNumInput *m_pitch;
  KIntNumInput *m_duration;
  QCheckBox    *m_useBell;
};

#endif
