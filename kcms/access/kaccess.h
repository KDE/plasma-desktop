#ifndef __K_ACCESS_H__
#define __K_ACCESS_H__


#include <QWidget>
#include <QColor>
//Added by qt3to4:
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
