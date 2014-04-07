/**
 * Copyright (c) 2000 Antonio Larrosa <larrosa@kde.org>
 * KDE Frameworks 5 port Copyright (C) 2013 Jonathan Riddell <jr@jriddell.org>
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
 */

#ifndef ICONTHEMES_H
#define ICONTHEMES_H

#include <KCModule>
#include <QLabel>
#include <QLoggingCategory>

class QStringList;
class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;

Q_DECLARE_LOGGING_CATEGORY(KCM_ICONS)

class IconThemesConfig : public KCModule
{
  Q_OBJECT

public:
  IconThemesConfig(QWidget *parent);
  virtual ~IconThemesConfig();

  void loadThemes();
  bool installThemes(const QStringList &themes, const QString &archiveName);
  QStringList findThemeDirs(const QString &archiveName);

  void updateRemoveButton();

  void load();
  void save();
  void defaults();

  int buttons();

protected Q_SLOTS:
  void themeSelected(QTreeWidgetItem *item);
  void installNewTheme();
  void getNewTheme();
  void removeSelectedTheme();

private:
  QTreeWidgetItem *iconThemeItem(const QString &name);

  QTreeWidget *m_iconThemes;
  QPushButton *m_removeButton;

  QLabel *m_previewExec;
  QLabel *m_previewFolder;
  QLabel *m_previewDocument;
  QTreeWidgetItem *m_defaultTheme;
  bool m_bChanged;
};

#endif // ICONTHEMES_H

