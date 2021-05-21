/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SWITCHING_TAB_H
#define SWITCHING_TAB_H

#include <QWidget>

#include <utils/d_ptr.h>

class QKeySequence;
class KCoreConfigSkeleton;

/**
 * SwitchingTab
 */
class SwitchingTab : public QWidget
{
    Q_OBJECT
public:
    explicit SwitchingTab(QWidget *parent);
    ~SwitchingTab() override;

    KCoreConfigSkeleton *mainConfig();

private Q_SLOTS:
    void shortcutChanged(const QKeySequence &sequence);

private:
    D_PTR;
};

#endif // SWITCHING_TAB_H
