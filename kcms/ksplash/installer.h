/***************************************************************************
 *   Copyright Ravikiran Rajagopal 2003                                    *
 *   ravi@ee.eng.ohio-state.edu                                            *
 *   Copyright (c) 1998 Stefan Taferner <taferner@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 ***************************************************************************/
#ifndef SPLASHINSTALLER_H
#define SPLASHINSTALLER_H

#include <QMap>
#include <QPoint>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QDropEvent>

#include <klistwidget.h>
#include <kurl.h>

class QLabel;
class QTextEdit;
class QPushButton;
class ThemeListBox;

class SplashInstaller : public QWidget
{
  Q_OBJECT
public:
  SplashInstaller(QWidget *parent=0, const char *aName=0, bool aInit=false);
  ~SplashInstaller();

  virtual void load();
  virtual void save();
  virtual void defaults();

Q_SIGNALS:
  void changed( bool state );

protected Q_SLOTS:
  virtual void slotNew();
  virtual void slotAdd();
  virtual void slotRemove();
  virtual void slotTest();
  virtual void slotSetTheme(int);
  void slotFilesDropped(const KUrl::List &urls);

protected:
  /** Scan Themes directory for available theme packages */
  virtual void readThemesList();
  /** add a theme to the list, returns the list index */
  int addTheme(const QString &path, const QString &name);
  void addNewTheme(const KUrl &srcURL);
  int findTheme( const QString &theme );

private:
  bool mGui;
  ThemeListBox *mThemesList;
  QString mEngineOfSelected;
  QPushButton *mBtnNew, *mBtnAdd, *mBtnRemove, *mBtnTest;
  QTextEdit *mText;
  QLabel *mPreview;
};

class ThemeListBox: public KListWidget
{
  Q_OBJECT
public:
  ThemeListBox(QWidget *parent);
  QMap<QString, QString> text2path;

Q_SIGNALS:
  void filesDropped(const KUrl::List &urls);

protected:
  void dragEnterEvent(QDragEnterEvent* event);
  void dragMoveEvent(QDragMoveEvent* event);
  void dropEvent(QDropEvent* event);
  void mouseMoveEvent(QMouseEvent *e);
  void mousePressEvent(QMouseEvent *e);

private:
  QString mDragFile;
  QPoint mOldPos;

};

#endif
