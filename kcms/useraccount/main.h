/**
 *  Copyright (C) 2015 Leslie Zhai <xiang.zhai@i-soft.com.cn>
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

#ifndef MAIN_H
#define MAIN_H

#include <kcmodule.h>
#include <kemailsettings.h>

#include <QListWidget>
#include <QLabel>

class KUser;
class QEvent;
class QObject;
class KUrl;

class KCMUserAccount : public KCModule
{
	Q_OBJECT

public:
	explicit KCMUserAccount(QWidget* parent, const QVariantList& list = QVariantList());
	~KCMUserAccount();

	/**
	 * The user data is loaded from  chfn(/etc/password) and then 
	 * written back as well as to KDE's own(KEmailSettings).
	 * The user won't notice this(assuming they change the KDE settings via 
	 * this KCM) and will make KDE play nice with environments which use 
	 * /etc/password.
	 */
	void load();

private slots:
    void slotItemClicked(QListWidgetItem* item);

private:
    QListWidget* _accountList;
    KUser* _ku;
    QLabel* _currentFaceIcon;
    QLabel* _currentFullName;
    QLabel* _currentAccountType;

    QIcon _faceIcon(QString faceIconPath);
    QPixmap _facePixmap(QString faceIconPath);
};

#endif // MAIN_H
