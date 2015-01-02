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

#ifndef __K_ACCESS_H__
#define __K_ACCESS_H__


#include <QWidget>
#include <QColor>
#include <QLabel>
#include <QPaintEvent>


#include <KUniqueApplication>
#include <KUrl>

#include <phonon/MediaObject>

#include <X11/Xlib.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#undef explicit
#include <fixx11h.h>

class KDialog;
class QLabel;
class KComboBox;

class KAccessApp : public KUniqueApplication
{
  Q_OBJECT

public:

  explicit KAccessApp(bool allowStyles=true, bool GUIenabled=true);

#if 0
  bool x11EventFilter(XEvent *event);
#endif

  int newInstance();

  void setXkbOpcode(int opcode);

protected:

  void readSettings();

  void xkbStateNotify();
  void xkbBellNotify(XkbBellNotifyEvent *event);
  void xkbControlsNotify(XkbControlsNotifyEvent *event);


private Q_SLOTS:

  void activeWindowChanged(WId wid);
  void notifyChanges();
  void applyChanges();
  void yesClicked();
  void noClicked();
  void dialogClosed();


private:
   void  createDialogContents();
   void  initMasks();

  int xkb_opcode;
  unsigned int features;
  unsigned int requestedFeatures;

  bool    _systemBell, _artsBell, _visibleBell, _visibleBellInvert;
  QColor  _visibleBellColor;
  int     _visibleBellPause;

  bool    _gestures, _gestureConfirmation;
  bool    _kNotifyModifiers, _kNotifyAccessX;

  QWidget *overlay;

  Phonon::MediaObject *_player;
  QString _currentPlayerSource;

  WId _activeWindow;

  KDialog *dialog;
  QLabel *featuresLabel;
  KComboBox *showModeCombobox;

  int keys[8];
  int state;
};


class VisualBell : public QWidget
{
  Q_OBJECT

public:

  VisualBell(int pause)
    : QWidget(( QWidget* )0, Qt::X11BypassWindowManagerHint), _pause(pause)
    {}


protected:

  void paintEvent(QPaintEvent *);


private:

  int _pause;

};




#endif
