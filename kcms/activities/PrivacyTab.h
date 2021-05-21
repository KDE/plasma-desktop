/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PRIVACY_TAB_H
#define PRIVACY_TAB_H

#include <QWidget>

#include <utils/d_ptr.h>

class KCoreConfigSkeleton;

/**
 * PrivacyTab
 */
class PrivacyTab : public QWidget
{
    Q_OBJECT
public:
    explicit PrivacyTab(QWidget *parent);
    ~PrivacyTab() override;

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
        NoApplications = 2,
    };

    D_PTR;
};

#endif // PRIVACY_TAB_H
