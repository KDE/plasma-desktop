/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QQuickWidget>

#include <utils/d_ptr.h>

/**
 * ActivitiesTab
 */
class ActivitiesTab : public QQuickWidget
{
    Q_OBJECT
public:
    explicit ActivitiesTab(QWidget *parent);
    ~ActivitiesTab() override;

private:
    D_PTR;
};
