/***************************************************************************
 *   Copyright (C) 2015 by Eike Hein <hein@kde.org>                   *
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

#ifndef TOOLBOXINTERFACE_H
#define TOOLBOXINTERFACE_H

#include <QObject>

class ToolboxInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool showToolbox READ showToolbox WRITE setShowToolbox NOTIFY showToolboxChanged)
    Q_PROPERTY(bool toolboxFound READ toolboxFound NOTIFY toolboxFoundChanged)
    Q_PROPERTY(QObject* appletInterface READ appletInterface WRITE setAppletInterface NOTIFY appletInterfaceChanged);

    public:
        ToolboxInterface(QObject *parent = 0);
        ~ToolboxInterface();

        bool showToolbox() const;
        void setShowToolbox(bool show);

        bool toolboxFound() const;

        QObject *appletInterface() const;
        void setAppletInterface(QObject *appletInterface);

        bool eventFilter(QObject *watched, QEvent *event);

    Q_SIGNALS:
        void showToolboxChanged() const;
        void toolboxFoundChanged() const;
        void appletInterfaceChanged() const;

    private:
        void setToolboxVisible(bool visible);

        bool m_showToolbox;
        bool m_toolboxFound;
        QObject *m_appletInterface;
};

#endif
