/*
 *  main.h
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
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
 *
 */
#ifndef main_included
#define main_included

#include <kcmodule.h>

class Dtime;
class QTabWidget;
class KProcess;


class KclockModule : public KCModule
{
  Q_OBJECT

public:
  KclockModule(QWidget *parent, const QVariantList &);

  void	save();
  void	load();

private:
  QTabWidget   *tab;
  Dtime	*dtime;
  KProcess *process;
};

#endif // main_included
