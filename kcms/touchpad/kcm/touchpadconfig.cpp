/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadconfig.h"

#include "../logging.h"
#include "touchpadbackend.h"
#include <config-build-options.h>

#include <KLocalizedContext>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KWindowSystem>

#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QVBoxLayout>

K_PLUGIN_CLASS_WITH_JSON(TouchpadConfig, "kcm_touchpad.json")

extern "C" {
Q_DECL_EXPORT void kcminit()
{
#if BUILD_KCM_TOUCHPAD_X11
    if (KWindowSystem::isPlatformX11()) {
        TouchpadConfig::kcmInit();
    }
#endif
}
}

TouchpadConfig::TouchpadConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    m_backend = TouchpadBackend::implementation();

    m_initError = !m_backend->errorString().isNull();

    m_view = new QQuickWidget(widget());

    QVBoxLayout *layout = new QVBoxLayout(widget());

    layout->addWidget(m_view);
    widget()->setLayout(layout);

    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_view->setClearColor(Qt::transparent);
    m_view->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_view->rootContext()->setContextProperty("backend", m_backend);
    m_view->rootContext()->setContextProperty("deviceModel", QVariant::fromValue(m_backend->getDevices().toList()));

    QObject::connect(m_view, &QQuickWidget::statusChanged, [&](QQuickWidget::Status status) {
        if (status == QQuickWidget::Ready) {
            connect(m_view->rootObject(), SIGNAL(changeSignal()), this, SLOT(onChange()));
        }
    });

    qmlRegisterSingletonInstance("org.kde.touchpad.kcm", 1, 0, "TouchpadConfig", this);

    m_view->engine()->rootContext()->setContextObject(new KLocalizedContext(m_view->engine()));
    m_view->setSource(QUrl("qrc:/libinput/touchpad.qml"));
    m_view->resize(QSize(500, 600));

    if (m_initError) {
        Q_EMIT showMessage(m_backend->errorString());
    } else {
        connect(m_backend, &TouchpadBackend::touchpadAdded, this, &TouchpadConfig::onTouchpadAdded);
        connect(m_backend, &TouchpadBackend::touchpadRemoved, this, &TouchpadConfig::onTouchpadRemoved);
    }

    setButtons(KCModule::Default | KCModule::Apply);
}

void TouchpadConfig::kcmInit()
{
#if BUILD_KCM_TOUCHPAD_X11
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (backend->getMode() == TouchpadInputBackendMode::XLibinput) {
        backend->getConfig();
        backend->applyConfig();
    }
#endif
}

void TouchpadConfig::load()
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

void TouchpadConfig::save()
{
    if (!m_backend->applyConfig()) {
        Q_EMIT showMessage(i18n("Not able to save all changes. See logs for more information. Please restart this configuration module and try again."));
    } else {
        Q_EMIT showMessage(QString());
    }

    // load newly written values
    load();
    // in case of error, config still in changed state
    setNeedsSave(m_backend->isChangedConfig());
}

void TouchpadConfig::defaults()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->getDefaultConfig()) {
        Q_EMIT showMessage(i18n("Error while loading default values. Failed to set some options to their default values."));
    }
    QMetaObject::invokeMethod(m_view->rootObject(), "syncValuesFromBackend");
    setNeedsSave(m_backend->isChangedConfig());
}

void TouchpadConfig::onChange()
{
    if (!m_backend->touchpadCount()) {
        return;
    }
    hideErrorMessage();
    setNeedsSave(m_backend->isChangedConfig());
}

void TouchpadConfig::onTouchpadAdded(bool success)
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

void TouchpadConfig::onTouchpadRemoved(int index)
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

    setNeedsSave(m_backend->isChangedConfig());
}

void TouchpadConfig::hideErrorMessage()
{
    Q_EMIT showMessage(QString());
}

#include "touchpadconfig.moc"
