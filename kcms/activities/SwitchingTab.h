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

#ifndef SWITCHING_TAB_H
#define SWITCHING_TAB_H

#include <QWidget>

#include <utils/d_ptr.h>

class QKeySequence;
class KCoreConfigSkeleton;

/**
 * SwitchingTab
 */
class SwitchingTab : public QWidget {
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
