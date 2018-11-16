/*
 *   Copyright 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TRASH_H
#define TRASH_H

#include <QObject>

class Trash : public QObject
{
    Q_OBJECT

public:
    explicit Trash(QObject *parent = nullptr);
    ~Trash() override = default;

    Q_INVOKABLE void trashUrls(const QList<QUrl> &urls);
    Q_INVOKABLE void emptyTrash();
    Q_INVOKABLE bool canBeTrashed(const QUrl &url) const;
    Q_INVOKABLE QList<QUrl> trashableUrls(const QList<QUrl> &urls) const;

};


#endif // TRASH_H
