/*
 * Copyright (c) 2019 Benjamin Port <benjamin.port@enioka.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_DESKTOP_ICONSSETTINGS_H
#define PLASMA_DESKTOP_ICONSSETTINGS_H

#include "iconssettingsbase.h"

class IconsSettings : public IconsSettingsBase {
    Q_OBJECT
public:
    IconsSettings(QObject *parent = nullptr);
    ~IconsSettings() override;
public slots:
    void updateIconTheme();
    void updateThemeDirty();
private:
    bool m_themeDirty;
};


#endif //PLASMA_DESKTOP_ICONSSETTINGS_H
