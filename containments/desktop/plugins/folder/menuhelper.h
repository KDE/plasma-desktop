/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MENUHELPER_H
#define MENUHELPER_H

#include <QObject>

class QAction;

class MenuHelper : public QObject
{
    Q_OBJECT

public:
    explicit MenuHelper(QObject *parent = nullptr);
    ~MenuHelper() override;

    Q_INVOKABLE QString iconName(QAction *action) const;

    Q_INVOKABLE void setMenu(QAction *action, QObject *menu);
};

#endif
