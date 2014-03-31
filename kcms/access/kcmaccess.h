/**
 * kcmaccess.h
 *
 * Copyright (c) 2000 Matthias H�zer-Klpfel <hoelzer@kde.org>
 *
 */

#ifndef __kcmaccess_h__
#define __kcmaccess_h__


#define KDE3_SUPPORT
#include <kcmodule.h>
#undef KDE3_SUPPORT
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
