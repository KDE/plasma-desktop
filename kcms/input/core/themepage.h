/*
 * Copyright (C) 2003 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __THEMEPAGE_H
#define __THEMEPAGE_H

class QTreeWidget;
class QTreeWidgetItem;

class ThemePage : public QWidget
{
	Q_OBJECT

	public:
		explicit ThemePage( QWidget* parent = 0, const char* name = 0 );
		~ThemePage();

		// Called by the KCM
		void save();
		void load();
		void defaults();

	Q_SIGNALS:
		void changed( bool );

	private Q_SLOTS:
		void selectionChanged( QTreeWidgetItem *,QTreeWidgetItem* );

	private:
		void insertThemes();
		void fixCursorFile();

		QTreeWidget *listview;
		QString currentTheme, selectedTheme;
};

#endif // __THEMEPAGE_H

// vim: set noet ts=4 sw=4:
