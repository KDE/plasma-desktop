/*
 *  Copyright (C) 2001 Rik Hemsley (rikkus) <rik@kde.org>
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
 */

#ifndef __kcmlaunch_h__
#define __kcmlaunch_h__

#include <kcmodule.h>

class QCheckBox;
class QComboBox;
class QLabel;

class QSpinBox;

class LaunchConfig : public KCModule
{
  Q_OBJECT

  public:

    explicit LaunchConfig(QWidget * parent = 0, const QVariantList &list = QVariantList() );

    virtual ~LaunchConfig();

    void load();
    void save();
    void defaults();

  protected Q_SLOTS:

    void checkChanged();
    void slotBusyCursor(int);
    void slotTaskbarButton(bool);

  protected:

    enum FeedbackStyle
    {
      BusyCursor            = 1 << 0,
      TaskbarButton         = 1 << 1,

      Default = BusyCursor | TaskbarButton
    };


  private:

    QLabel    * lbl_cursorTimeout;
    QLabel    * lbl_taskbarTimeout;
    QComboBox * cb_busyCursor;
    QCheckBox * cb_taskbarButton;
    QSpinBox * sb_cursorTimeout;
    QSpinBox * sb_taskbarTimeout;

};

#endif
