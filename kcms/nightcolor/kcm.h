/********************************************************************
Copyright 2017 Roman Gilg <subdiff@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#ifndef _KCM_NIGHTCOLOR_H
#define _KCM_NIGHTCOLOR_H

#include <KQuickAddons/ConfigModule>

namespace ColorCorrect {

class KCMNightColor : public KQuickAddons::ConfigModule
{
    Q_OBJECT
public:
    KCMNightColor(QObject* parent, const QVariantList& args);
    ~KCMNightColor() override {}

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

Q_SIGNALS:
    void loadRelay();
    void saveRelay();
    void defaultsRelay();
};

}

#endif  // _KCM_NIGHTCOLOR_H
