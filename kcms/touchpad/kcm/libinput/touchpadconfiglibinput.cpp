/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadconfiglibinput.h"

#include <KAboutData>
#include <KLocalizedString>
#include <kdeclarative/kdeclarative.h>

#include <QMetaObject>
#include <QQmlContext>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "../touchpadconfigcontainer.h"
#include "touchpadbackend.h"

#include "plasma_version.h"

TouchpadConfigLibinput::TouchpadConfigLibinput(TouchpadConfigContainer *parent, TouchpadBackend *backend, const QVariantList & /*args*/)
    : TouchpadConfigPlugin(parent, backend)
{
    KAboutData *data = new KAboutData(QStringLiteral("kcm_touchpad"),
                                      i18n("Touchpad KCM"),
                                      PLASMA_VERSION_STRING,
                                      i18n("System Settings module for managing your touchpad"),
                                      KAboutLicense::GPL_V2,
                                      i18n("Copyright © 2016 Roman Gilg"),
                                      QString());

    data->addAuthor(i18n("Roman Gilg"), i18n("Developer"), QStringLiteral("subdiff@gmail.com"));

    m_parent->setAboutData(data);

    m_initError = !m_backend->errorString().isNull();

    m_view = new QQuickWidget(this);

    QVBoxLayout *layout = new QVBoxLayout(parent);

    layout->addWidget(m_view);
    parent->setLayout(layout);

    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_view->setClearColor(Qt::transparent);
    m_view->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_view->rootContext()->setContextProperty("backend", m_backend);
    m_view->rootContext()->setContextProperty("deviceModel", QVariant::fromValue(m_backend->getDevices().toList()));

    qmlRegisterSingletonInstance("org.kde.touchpad.kcm", 1, 0, "TouchpadConfig", this);

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(m_view->engine());
    kdeclarative.setupBindings();
    m_view->setSource(QUrl("qrc:/libinput/touchpad.qml"));

    if (m_initError) {
        Q_EMIT showMessage(m_backend->errorString());
    } else {
        connect(m_backend, SIGNAL(touchpadAdded(bool)), this, SLOT(onTouchpadAdded(bool)));
        connect(m_backend, SIGNAL(touchpadRemoved(int)), this, SLOT(onTouchpadRemoved(int)));
        connect(m_view->rootObject(), SIGNAL(changeSignal()), this, SLOT(onChange()));
    }

    m_view->show();
}

QSize TouchpadConfigLibinput::sizeHint() const
{
    return QQmlProperty::read(m_view->rootObject(), "sizeHint").toSize();
}

QSize TouchpadConfigLibinput::minimumSizeHint() const
{
    return QQmlProperty::read(m_view->rootObject(), "minimumSizeHint").toSize();
}

void TouchpadConfigLibinput::load()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->getConfig()) {
        Q_EMIT showMessage(i18n("Error while loading values. See logs for more information. Please restart this configuration module."));
    } else {
        if (!m_backend->touchpadCount()) {
            Q_EMIT showMessage(i18n("No touchpad found. Connect touchpad now."));
        }
    }
    QMetaObject::invokeMethod(m_view->rootObject(), "syncValuesFromBackend");
}

void TouchpadConfigLibinput::save()
{
    if (!m_backend->applyConfig()) {
        Q_EMIT showMessage(i18n("Not able to save all changes. See logs for more information. Please restart this configuration module and try again."));
    } else {
        Q_EMIT showMessage(QString());
    }

    // load newly written values
    load();
    // in case of error, config still in changed state
    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
}

void TouchpadConfigLibinput::defaults()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->getDefaultConfig()) {
        Q_EMIT showMessage(i18n("Error while loading default values. Failed to set some options to their default values."));
    }
    QMetaObject::invokeMethod(m_view->rootObject(), "syncValuesFromBackend");
    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
}

void TouchpadConfigLibinput::onChange()
{
    if (!m_backend->touchpadCount()) {
        return;
    }
    hideErrorMessage();
    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
}

void TouchpadConfigLibinput::onTouchpadAdded(bool success)
{
    QQuickItem *rootObj = m_view->rootObject();

    if (!success) {
        Q_EMIT showMessage(i18n("Error while adding newly connected device. Please reconnect it and restart this configuration module."));
    }

    int activeIndex;
    if (m_backend->touchpadCount() == 1) {
        // if no touchpad was connected previously, show the new device and hide the no-device-message
        activeIndex = 0;
        hideErrorMessage();
    } else {
        activeIndex = QQmlProperty::read(rootObj, "deviceIndex").toInt();
    }
    m_view->rootContext()->setContextProperty("deviceModel", QVariant::fromValue(m_backend->getDevices()));
    QMetaObject::invokeMethod(rootObj, "resetModel", Q_ARG(QVariant, activeIndex));
    QMetaObject::invokeMethod(rootObj, "syncValuesFromBackend");
}

void TouchpadConfigLibinput::onTouchpadRemoved(int index)
{
    QQuickItem *rootObj = m_view->rootObject();

    int activeIndex = QQmlProperty::read(rootObj, "deviceIndex").toInt();
    if (activeIndex == index) {
        if (m_backend->touchpadCount()) {
            Q_EMIT showMessage(i18n("Touchpad disconnected. Closed its setting dialog."), 0 /*Kirigami.MessageType.Information*/);
        } else {
            Q_EMIT showMessage(i18n("Touchpad disconnected. No other touchpads found."), 0 /*Kirigami.MessageType.Information*/);
        }
        activeIndex = 0;
    } else {
        if (index < activeIndex) {
            activeIndex--;
        }
    }
    m_view->rootContext()->setContextProperty("deviceModel", QVariant::fromValue(m_backend->getDevices()));
    QMetaObject::invokeMethod(m_view->rootObject(), "resetModel", Q_ARG(QVariant, activeIndex));
    QMetaObject::invokeMethod(rootObj, "syncValuesFromBackend");

    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
}

void TouchpadConfigLibinput::hideErrorMessage()
{
    Q_EMIT showMessage(QString());
}
