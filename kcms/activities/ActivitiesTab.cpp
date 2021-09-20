/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ActivitiesTab.h"

#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>

#include <QGuiApplication>
#include <QQuickView>
#include <QVBoxLayout>

#include <KLocalizedContext>

#include "ExtraActivitiesInterface.h"
#include "definitions.h"

#include <utils/d_ptr_implementation.h>

#include "kactivities-kcm-features.h"

class ActivitiesTab::Private
{
public:
    ExtraActivitiesInterface *extraActivitiesInterface;
};

ActivitiesTab::ActivitiesTab(QWidget *parent)
    : QQuickWidget(parent)
    , d()
{
    setClearColor(QGuiApplication::palette().window().color());
    setResizeMode(QQuickWidget::SizeRootObjectToView);
    rootContext()->setContextProperty(QStringLiteral("kactivitiesExtras"), d->extraActivitiesInterface);
    engine()->rootContext()->setContextObject(new KLocalizedContext(this));
    setSource(QUrl::fromLocalFile(KAMD_KCM_DATADIR + QStringLiteral("/qml/activitiesTab/main.qml")));
}

ActivitiesTab::~ActivitiesTab()
{
}
