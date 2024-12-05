/*
 *  SPDX-FileCopyrightText: 2015-2016 Ivan Cukic <ivan.cukic@kde.org>
 *  SPDX-FileCopyrightText: 2023 Ismael Asensio <isma.af@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "kcm_activities.h"
#include "activityconfig.h"

#include <KAuthorized>
#include <KLocalizedString>
#include <KPluginFactory>
#include <PlasmaActivities/Controller>

#include <QTimer>
#include <QtQml/QQmlModuleRegistration>

K_PLUGIN_CLASS_WITH_JSON(ActivitiesModule, "kcm_activities.json")

ActivitiesModule::ActivitiesModule(QObject *parent, const KPluginMetaData &metaData, const QVariantList &args)
    : KQuickConfigModule(parent, metaData)
    , m_isNewActivityAuthorized(KAuthorized::authorize(QStringLiteral("plasma-desktop/add_activities")))
{
    qmlRegisterType<ActivityConfig>("org.kde.kcms.activities", 1, 0, "ActivityConfig");

    if (!args.isEmpty()) {
        m_firstArgument = args.first().toString();
    }

    connect(this, &KAbstractConfigModule::activationRequested, this, [this](const QVariantList &args) {
        if (!args.isEmpty()) {
            handleArgument(args.first().toString());
        }
    });
}

ActivitiesModule::~ActivitiesModule()
{
}

bool ActivitiesModule::isNewActivityAuthorized() const
{
    return m_isNewActivityAuthorized;
}

void ActivitiesModule::configureActivity(const QString &id)
{
    if (!id.isEmpty() && !KActivities::Controller().activities().contains(id)) {
        qWarning() << "Cannot configure activity. There is no activity with id" << id;
        qWarning() << "List of Activities: " << KActivities::Controller().activities();
        return;
    }

    if (depth() > 1) {
        pop();
    }
    push(QStringLiteral("ActivityEditor.qml"), QVariantMap{{QStringLiteral("activityId"), id}});
}

void ActivitiesModule::newActivity()
{
    // Launch the editor with an empty string as activity id
    configureActivity(QString());
}

void ActivitiesModule::deleteActivity(const QString &id)
{
    if (!m_isNewActivityAuthorized) {
        return;
    }

    KActivities::Controller().removeActivity(id);
}

void ActivitiesModule::load()
{
    if (m_firstArgument.isEmpty()) {
        return;
    }

    // Delay so the KActivities::Consumer can load the activities status
    QTimer::singleShot(0, this, [this]() {
        handleArgument(m_firstArgument);
        m_firstArgument = QString();
    });
}

void ActivitiesModule::handleArgument(const QString &argument)
{
    if (argument == QStringLiteral("newActivity")) {
        newActivity();
    } else {
        configureActivity(argument);
    }
}

#include "kcm_activities.moc"

#include "moc_kcm_activities.cpp"
