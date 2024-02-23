/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "libinput_config.h"
#include "../configcontainer.h"

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KMessageWidget>

#include <QMetaObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQmlProperty>
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>

#include "inputbackend.h"

static QVariant getDeviceList(InputBackend *backend)
{
    return QVariant::fromValue(backend->getDevices().toList());
}

LibinputConfig::LibinputConfig(ConfigContainer *parent, InputBackend *backend)
    : QWidget(parent->widget())
    , m_parent(parent)
{
    m_backend = backend;

    m_initError = !m_backend->errorString().isNull();

    m_view = new QQuickWidget(this);

    m_errorMessage = new KMessageWidget(this);
    m_errorMessage->setCloseButtonVisible(false);
    m_errorMessage->setWordWrap(true);
    m_errorMessage->setVisible(false);

    QVBoxLayout *layout = new QVBoxLayout(parent->widget());

    layout->addWidget(m_errorMessage);
    layout->addWidget(m_view);
    parent->widget()->setLayout(layout);

    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_view->setClearColor(Qt::transparent);
    m_view->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_view->rootContext()->setContextProperty("backend", m_backend);
    m_view->rootContext()->setContextProperty("deviceModel", getDeviceList(m_backend));

    QObject::connect(m_view, &QQuickWidget::statusChanged, [&](QQuickWidget::Status status) {
        if (status == QQuickWidget::Ready) {
            connect(m_view->rootObject(), SIGNAL(changeSignal()), this, SLOT(onChange()));
        }
    });

    m_view->engine()->rootContext()->setContextObject(new KLocalizedContext(m_view->engine()));

#if BUILD_KCM_MOUSE_X11
    bool deviceless = m_backend->mode() == InputBackendMode::XLibinput;
#else
    bool deviceless = false;
#endif
    if (deviceless) {
        m_view->setSource(QUrl(QStringLiteral("qrc:/ui/main_deviceless.qml")));
    } else {
        m_view->setSource(QUrl(QStringLiteral("qrc:/ui/main.qml")));
    }

    if (m_initError) {
        m_errorMessage->setMessageType(KMessageWidget::Error);
        m_errorMessage->setText(m_backend->errorString());
        QMetaObject::invokeMethod(m_errorMessage, "animatedShow", Qt::QueuedConnection);
    } else {
        connect(m_backend, SIGNAL(deviceAdded(bool)), this, SLOT(onDeviceAdded(bool)));
        connect(m_backend, SIGNAL(deviceRemoved(int)), this, SLOT(onDeviceRemoved(int)));
    }

    // Just set it to a reasonable default size
    // This only matters for kcmshell and most of the time we have the KCM in systemsettings open
    m_view->resize(QSize(300, 300));
    m_view->show();
}

void LibinputConfig::load()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->getConfig()) {
        m_errorMessage->setMessageType(KMessageWidget::Error);
        m_errorMessage->setText(i18n("Error while loading values. See logs for more information. Please restart this configuration module."));
        m_errorMessage->animatedShow();
    } else {
        if (!m_backend->deviceCount()) {
            m_errorMessage->setMessageType(KMessageWidget::Information);
            m_errorMessage->setText(i18n("No pointer device found. Connect now."));
            m_errorMessage->animatedShow();
        }
    }
    QMetaObject::invokeMethod(m_view->rootObject(), "syncValuesFromBackend");
}

void LibinputConfig::save()
{
    if (!m_backend->applyConfig()) {
        m_errorMessage->setMessageType(KMessageWidget::Error);
        m_errorMessage->setText(i18n("Not able to save all changes. See logs for more information. Please restart this configuration module and try again."));
        m_errorMessage->animatedShow();
    } else {
        hideErrorMessage();
    }
    // load newly written values
    load();
    // in case of error, config still in changed state
    m_parent->setNeedsSave(m_backend->isChangedConfig());
}

void LibinputConfig::defaults()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->getDefaultConfig()) {
        m_errorMessage->setMessageType(KMessageWidget::Error);
        m_errorMessage->setText(i18n("Error while loading default values. Failed to set some options to their default values."));
        m_errorMessage->animatedShow();
    }
    QMetaObject::invokeMethod(m_view->rootObject(), "syncValuesFromBackend");
    m_parent->setNeedsSave(m_backend->isChangedConfig());
}

void LibinputConfig::onChange()
{
    if (!m_backend->deviceCount()) {
        return;
    }
    hideErrorMessage();
    m_parent->setNeedsSave(m_backend->isChangedConfig());
}

void LibinputConfig::onDeviceAdded(bool success)
{
    QQuickItem *rootObj = m_view->rootObject();

    if (!success) {
        m_errorMessage->setMessageType(KMessageWidget::Error);
        m_errorMessage->setText(i18n("Error while adding newly connected device. Please reconnect it and restart this configuration module."));
    }

    int activeIndex;
    if (m_backend->deviceCount() == 1) {
        // if no pointer device was connected previously, show the new device and hide the no-device-message
        activeIndex = 0;
        hideErrorMessage();
    } else {
        activeIndex = QQmlProperty::read(rootObj, "deviceIndex").toInt();
    }
    m_view->rootContext()->setContextProperty("deviceModel", getDeviceList(m_backend));
    QMetaObject::invokeMethod(rootObj, "resetModel", Q_ARG(QVariant, activeIndex));
    QMetaObject::invokeMethod(rootObj, "syncValuesFromBackend");
}

void LibinputConfig::onDeviceRemoved(int index)
{
    QQuickItem *rootObj = m_view->rootObject();

    int activeIndex = QQmlProperty::read(rootObj, "deviceIndex").toInt();
    if (activeIndex == index) {
        m_errorMessage->setMessageType(KMessageWidget::Information);
        if (m_backend->deviceCount()) {
            m_errorMessage->setText(i18n("Pointer device disconnected. Closed its setting dialog."));
        } else {
            m_errorMessage->setText(i18n("Pointer device disconnected. No other devices found."));
        }
        m_errorMessage->animatedShow();
        activeIndex = 0;
    } else {
        if (index < activeIndex) {
            activeIndex--;
        }
    }
    m_view->rootContext()->setContextProperty("deviceModel", getDeviceList(m_backend));
    QMetaObject::invokeMethod(m_view->rootObject(), "resetModel", Q_ARG(QVariant, activeIndex));
    QMetaObject::invokeMethod(rootObj, "syncValuesFromBackend");

    m_parent->setNeedsSave(m_backend->isChangedConfig());
}

void LibinputConfig::hideErrorMessage()
{
    if (m_errorMessage->isVisible()) {
        m_errorMessage->animatedHide();
    }
}
