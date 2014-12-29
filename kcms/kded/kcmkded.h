/* This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 */
#ifndef KCMKDED_H
#define KCMKDED_H

#include <QLoggingCategory>

#include <KCModule>

class QPushButton;
class QTreeWidget;
class QTreeWidgetItem;
class KConfig;

Q_DECLARE_LOGGING_CATEGORY(KCM_KDED)

class KDEDConfig : public KCModule
{
Q_OBJECT
public:
	explicit KDEDConfig(QWidget* parent, const QVariantList& foo = QVariantList());
	~KDEDConfig() {}

	void       load();
	void       save();
	void       defaults();

protected Q_SLOTS:
	void slotReload();
	void slotStartService();
	void slotStopService();
	void slotServiceRunningToggled();
	void slotStartupItemSelected();
	void slotLodItemSelected();
	void slotItemChecked(QTreeWidgetItem *item, int column);
	void getServiceStatus();

        bool autoloadEnabled(KConfig *config, const QString &filename);
        void setAutoloadEnabled(KConfig *config, const QString &filename, bool b);

private:
	QTreeWidget *_lvLoD;
	QTreeWidget *_lvStartup;
	QPushButton *_pbStart;
	QPushButton *_pbStop;
	
	QString RUNNING;
	QString NOT_RUNNING;
};

#endif // KCMKDED_H

