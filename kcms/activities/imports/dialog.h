/*
    SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QKeySequence>

#include "utils/d_ptr.h"

class Dialog : public QDialog
{
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
