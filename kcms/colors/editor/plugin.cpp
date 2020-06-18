/*
 * Copyright 2014  Martin Gräßlin <mgraesslin@kde.org>
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
// kwin/kcmkwin/kwindecoration/declarative-plugin/plugin.cpp

#include "plugin.h"
#include "buttonsmodel.h"
#include "previewbutton.h"
#include "previewbridge.h"
#include "previewclient.h"
#include "previewitem.h"
#include "previewsettings.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationShadow>

namespace KDecoration2
{
namespace Preview
{

void Plugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.kcolorschemeeditor.private.kdecoration"));
    qmlRegisterType<KDecoration2::Preview::BridgeItem>(uri, 1, 0, "Bridge");
    qmlRegisterType<KDecoration2::Preview::Settings>(uri, 1, 0, "Settings");
    qmlRegisterType<KDecoration2::Preview::PreviewItem>(uri, 1, 0, "Decoration");
    qmlRegisterType<KDecoration2::Preview::PreviewButtonItem>(uri, 1, 0, "Button");
    qmlRegisterType<KDecoration2::Preview::ButtonsModel>(uri, 1, 0, "ButtonsModel");
    qmlRegisterAnonymousType<KDecoration2::Preview::PreviewClient>(uri, 1);
    qmlRegisterAnonymousType<KDecoration2::Decoration>(uri, 1);
    qmlRegisterAnonymousType<KDecoration2::DecorationShadow>(uri, 1);
    qmlRegisterAnonymousType<KDecoration2::Preview::PreviewBridge>(uri, 1);
}

}
}


