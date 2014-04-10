/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef SYSTEMSETTINGS_H
#define SYSTEMSETTINGS_H

#include <QObject>

class QWidget;

class SystemSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool singleClick READ singleClick NOTIFY singleClickChanged)
    Q_PROPERTY(int doubleClickInterval READ doubleClickInterval)

    public:
        SystemSettings(QObject *parent = 0);
        ~SystemSettings();

        bool singleClick() const;
        int doubleClickInterval() const; // FIXME TODO: Make notifyable.

        bool eventFilter(QObject *watched, QEvent *event);

    Q_SIGNALS:
        void singleClickChanged();

    private:
        // Keeping our own widget around is ugly, but beats filtering all
        // events on something busy like the main window.
        QWidget *m_monitoredWidget;

};

#endif
