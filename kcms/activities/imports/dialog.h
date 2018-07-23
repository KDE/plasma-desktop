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

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QKeySequence>

#include "utils/d_ptr.h"

class Dialog: public QDialog {
    Q_OBJECT

public:
    ~Dialog() override;

    static void showDialog(const QString &activityId = QString());

    void init(const QString &activityId = QString());

    QString activityId() const;
    void setActivityId(const QString &activityId);

    QString activityName() const;
    void setActivityName(const QString &activityName);

    QString activityDescription() const;
    void setActivityDescription(const QString &activityDescription);

    QString activityIcon() const;
    void setActivityIcon(const QString &activityIcon);

    QString activityWallpaper() const;
    void setActivityWallpaper(const QString &activityWallpaper);

    bool activityIsPrivate() const;
    void setActivityIsPrivate(bool activityIsPrivate);

    QKeySequence activityShortcut() const;
    void setActivityShortcut(const QKeySequence &activityShortcut);

public Q_SLOTS:
    void save();
    void create();
    void saveChanges(const QString &activityId);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Dialog(QObject *parent = nullptr);

    D_PTR;

};

#endif // DIALOG_H

