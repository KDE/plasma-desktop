/*
 *   Copyright (C) 2012 - 2016 by Ivan Cukic <ivan.cukic@kde.org>
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

#ifndef PRIVACY_TAB_H
#define PRIVACY_TAB_H

#include <QWidget>

#include <utils/d_ptr.h>

class KCoreConfigSkeleton;

/**
 * PrivacyTab
 */
class PrivacyTab : public QWidget {
    Q_OBJECT
public:
    explicit PrivacyTab(QWidget *parent);
    ~PrivacyTab() override;

    bool isDefault();
    KCoreConfigSkeleton *pluginConfig();
    
public Q_SLOTS:
    void defaults();
    void load();
    void save();

private Q_SLOTS:
    void forget(int count, const QString &what);
    void forgetLastHour();
    void forgetTwoHours();
    void forgetDay();
    void forgetAll();

    void spinKeepHistoryValueChanged(int value);

Q_SIGNALS:
    void blackListModelChanged(bool changed);
    void blackListModelDefaulted(bool isDefault);

private:
    enum WhatToRemember {
        AllApplications = 0,
        SpecificApplications = 1,
        NoApplications = 2
    };

    D_PTR;
};

#endif // PRIVACY_TAB_H
