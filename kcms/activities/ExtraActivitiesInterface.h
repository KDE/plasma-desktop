/*
 *   Copyright (C) 2015 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2 of
 *   the License or (at your option) version 3 or any later version
 *   accepted by the membership of KDE e.V. (or its successor approved
 *   by the membership of KDE e.V.), which shall act as a proxy
 *   defined in Section 14 of version 3 of the license.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXTRA_ACTIVITIES_INTERFACE_H
#define EXTRA_ACTIVITIES_INTERFACE_H

#include <QAbstractListModel>

#include <utils/d_ptr.h>

#include <QJSValue>
#include <QKeySequence>

class ExtraActivitiesInterface : public QObject {
    Q_OBJECT

public:
    explicit ExtraActivitiesInterface(QObject *parent = nullptr);
    ~ExtraActivitiesInterface() override;

public Q_SLOTS:
    void setIsPrivate(const QString &activity, bool isPrivate,
                      QJSValue callback);
    void getIsPrivate(const QString &activity, QJSValue callback);

    void setShortcut(const QString &activity, const QKeySequence &keySequence);
    QKeySequence shortcut(const QString &activity);

private:
    D_PTR;
};

#endif // EXTRA_ACTIVITIES_INTERFACE_H
