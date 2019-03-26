/*
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

#include "kcm.h"

#include <QGuiApplication>
#include <QStandardPaths>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>

#include <KActivities/ActivitiesModel>

#include <algorithm>

#include "sourcesmodel.h"
#include "filterproxymodel.h"

#include <notificationmanager/settings.h>

K_PLUGIN_FACTORY_WITH_JSON(KCMNotificationsFactory, "kcm_notifications.json", registerPlugin<KCMNotifications>();)

KCMNotifications::KCMNotifications(QObject *parent, const QVariantList &args)
    : KQuickAddons::ConfigModule(parent, args)
    , m_sourcesModel(new SourcesModel(this))
    , m_filteredModel(new FilterProxyModel(this))
    , m_settings(new NotificationManager::Settings(this))
    , m_activitiesModel(new KActivities::ActivitiesModel(this))
{
    const char uri[] = "org.kde.private.kcms.notifications";
    qmlRegisterUncreatableType<SourcesModel>(uri, 1, 0, "SourcesModel",
                                             QStringLiteral("Cannot create instances of SourcesModel"));
    qmlRegisterType<FilterProxyModel>();
    qmlProtectModule(uri, 1);

    qmlRegisterType<KActivities::ActivitiesModel>();

    KAboutData *about = new KAboutData(QStringLiteral("kcm_notifications"), i18n("Notifications"),
                                       QStringLiteral("5.0"), QString(), KAboutLicense::GPL);
    about->addAuthor(i18n("Kai Uwe Broulik"), QString(), QStringLiteral("kde@privat.broulik.de"));
    setAboutData(about);

    m_filteredModel->setSourceModel(m_sourcesModel);
}

KCMNotifications::~KCMNotifications()
{

}

SourcesModel *KCMNotifications::sourcesModel() const
{
    return m_sourcesModel;
}

FilterProxyModel *KCMNotifications::filteredModel() const
{
    return m_filteredModel;
}

NotificationManager::Settings *KCMNotifications::settings() const
{
    return m_settings;
}

KActivities::ActivitiesModel *KCMNotifications::activitiesModel() const
{
    return m_activitiesModel;
}

void KCMNotifications::load()
{
    m_settings->load();
    m_sourcesModel->load();

    //m_config->markAsClean();
    //m_config->reparseConfiguration();
}

void KCMNotifications::save()
{
    m_settings->save();
    //setNeedsSave(false);
}

void KCMNotifications::defaults()
{
    m_settings->defaults();
    //setNeedsSave(true);
}

#include "kcm.moc"
