/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#ifndef STATUSNOTIFIERTEST_H
#define STATUSNOTIFIERTEST_H

#include <QDialog>
#include <QObject>
#include <QWidget>

#include <KJob>

#include "ui_waylandtest.h"

class WaylandTestPrivate;

class WaylandTest : public QDialog, public Ui_WaylandTest
{
    Q_OBJECT

    public:
        WaylandTest(QWidget* parent = 0);
        virtual ~WaylandTest();

        void init();
        void log(const QString &msg);

    public Q_SLOTS:
        void updateUi();

    private:
        WaylandTestPrivate* d;
};

#endif
