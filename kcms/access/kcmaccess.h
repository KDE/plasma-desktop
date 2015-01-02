/*
    Copyright 2000 Matthias Hölzer-Klüpfel <hoelzer@kde.org>
    Copyright 2014 Frederik Gladhorn <gladhorn@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __kcmaccess_h__
#define __kcmaccess_h__


#include <kcmodule.h>
#include <knuminput.h>


class QCheckBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class KColorButton;
class QSlider;

class KAccessConfig : public KCModule
{
  Q_OBJECT

public:

  KAccessConfig( QWidget *parent, const QVariantList& );
  virtual ~KAccessConfig();

  void load();
  void save();
  void defaults();

protected Q_SLOTS:

  void configChanged();
  void checkAccess();
  void invertClicked();
  void flashClicked();
  void selectSound();
  void changeFlashScreenColor();
  void configureKNotify();

private:

  QCheckBox *systemBell, *customBell, *visibleBell;
  QRadioButton *invertScreen, *flashScreen;
  QLabel    *soundLabel, *colorLabel;
  QLineEdit *soundEdit;
  QPushButton *soundButton;
  KColorButton *colorButton;
  KDoubleNumInput *durationSlider;

  QCheckBox *stickyKeys, *stickyKeysLock, *stickyKeysAutoOff;
  QCheckBox *stickyKeysBeep, *toggleKeysBeep, *kNotifyModifiers;
  QPushButton *kNotifyModifiersButton;

  QCheckBox *slowKeys, *bounceKeys;
  KDoubleNumInput *slowKeysDelay, *bounceKeysDelay;
  QCheckBox *slowKeysPressBeep, *slowKeysAcceptBeep;
  QCheckBox *slowKeysRejectBeep, *bounceKeysRejectBeep;

  QCheckBox *gestures, *gestureConfirmation;
  QCheckBox *timeout;
  KIntNumInput *timeoutDelay;
  QCheckBox *accessxBeep, *kNotifyAccessX;
  QPushButton *kNotifyAccessXButton;
};


#endif
