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
#include "kcm.h"

#include <KPluginFactory>
#include <KPluginLoader>
#include <KAboutData>
#include <KLocalizedString>

K_PLUGIN_FACTORY_WITH_JSON(KCMNightColorFactory, "kcm_nightcolor.json", registerPlugin<ColorCorrect::KCMNightColor>();)

namespace ColorCorrect {

KCMNightColor::KCMNightColor(QObject *parent, const QVariantList& args)
    : KQuickAddons::ConfigModule(parent, args)
{
    KAboutData* about = new KAboutData(QStringLiteral("kcm_nightcolor"), i18n("Night Color"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Roman Gilg"), QString(), QStringLiteral("subdiff@gmail.com"));
    setAboutData(about);
    setButtons(Apply | Default);
}

void KCMNightColor::load()
{
    emit loadRelay();
    setNeedsSave(false);
}

void KCMNightColor::defaults()
{
    emit defaultsRelay();
}

void KCMNightColor::save()
{
    emit saveRelay();
}

}

#include "kcm.moc"
