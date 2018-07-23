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

#ifndef MAIN_CONFIGURATION_WIDGET_H
#define MAIN_CONFIGURATION_WIDGET_H

#include <KCModule>
#include <KPluginFactory>
#include <KActivities/Consumer>
#include <KActivities/Info>

#include <utils/d_ptr.h>

/**
 * MainConfigurationWidget
 */
class MainConfigurationWidget : public KCModule {
    Q_OBJECT
public:
    MainConfigurationWidget(QWidget *parent, QVariantList args);
    ~MainConfigurationWidget() override;

public Q_SLOTS:
    void defaults() override;
    void load() override;
    void save() override;

private:
    D_PTR;
};

#endif // MAIN_CONFIGURATION_WIDGET_H
