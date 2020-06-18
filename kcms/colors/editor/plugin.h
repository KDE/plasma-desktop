/*
 * Copyright 2014 Martin Gräßlin <mgraesslin@kde.org>
 * Copyright 2020 Noah Davis <noahadvs@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Copied from the Window Decorations KCM
// kwin/kcmkwin/kwindecoration/declarative-plugin/plugin.h

#ifndef KDECOARTIONS_PREVIEW_PLUGIN_H
#define KDECOARTIONS_PREVIEW_PLUGIN_H

#include <QQmlExtensionPlugin>

namespace KDecoration2
{
namespace Preview
{

class Plugin : public QQmlExtensionPlugin
{
    Q_PLUGIN_METADATA(IID "org.kde.kdecoration2")
    Q_OBJECT
public:
    void registerTypes(const char *uri) override;
};

}
}

#endif
