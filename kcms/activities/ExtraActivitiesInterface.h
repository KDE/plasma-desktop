/*
    SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef EXTRA_ACTIVITIES_INTERFACE_H
#define EXTRA_ACTIVITIES_INTERFACE_H

#include <QAbstractListModel>

#include <utils/d_ptr.h>

#include <QJSValue>
#include <QKeySequence>

class ExtraActivitiesInterface : public QObject
{
    Q_OBJECT

public:
    explicit ExtraActivitiesInterface(QObject *parent = nullptr);
    ~ExtraActivitiesInterface() override;

public Q_SLOTS:
    void setIsPrivate(const QString &activity, bool isPrivate, QJSValue callback);
    void getIsPrivate(const QString &activity, QJSValue callback);

    void setShortcut(const QString &activity, const QKeySequence &keySequence);
    QKeySequence shortcut(const QString &activity);

private:
    D_PTR;
};

#endif // EXTRA_ACTIVITIES_INTERFACE_H
