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

#ifndef VIEWPROPERTIESMENU_H
#define VIEWPROPERTIESMENU_H

#include <QObject>

class QAction;
class QActionGroup;
class QMenu;

class ViewPropertiesMenu : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QObject* menu READ menu CONSTANT);

    Q_PROPERTY(int arrangement READ arrangement WRITE setArrangement NOTIFY arrangementChanged)
    Q_PROPERTY(int alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)
    Q_PROPERTY(bool locked READ locked WRITE setLocked NOTIFY lockedChanged)
    Q_PROPERTY(int sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
    Q_PROPERTY(bool sortDesc READ sortDesc WRITE setSortDesc NOTIFY sortDescChanged)
    Q_PROPERTY(bool sortDirsFirst READ sortDirsFirst WRITE setSortDirsFirst NOTIFY sortDirsFirstChanged)

    public:
        ViewPropertiesMenu(QObject *parent = 0);
        ~ViewPropertiesMenu();

        QObject* menu() const;

        int arrangement() const;
        void setArrangement(int arrangement);

        int alignment() const;
        void setAlignment(int alignment);

        bool locked() const;
        void setLocked(bool locked);

        int sortMode() const;
        void setSortMode(int sortMode);

        bool sortDesc() const;
        void setSortDesc(bool sortDesc);

        bool sortDirsFirst() const;
        void setSortDirsFirst(bool sortDirsFirst);

    Q_SIGNALS:
        void arrangementChanged() const;
        void alignmentChanged() const;
        void lockedChanged() const;
        void sortModeChanged() const;
        void sortDescChanged() const;
        void sortDirsFirstChanged() const;

    private:
        QMenu *m_menu;
        QActionGroup *m_arrangement;
        QActionGroup *m_alignment;
        QActionGroup *m_sortMode;
        QAction *m_sortDesc;
        QAction *m_sortDirsFirst;
        QAction *m_locked;
};

#endif
