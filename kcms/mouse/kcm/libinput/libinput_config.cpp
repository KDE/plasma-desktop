/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "libinput_config.h"
#include "../configcontainer.h"

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageWidget>
#include <kdeclarative/kdeclarative.h>

#include <QMetaObject>
#include <QQmlContext>
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
    : ConfigPlugin(parent)
{
    m_backend = backend;

    KAboutData *data = new KAboutData(QStringLiteral("kcmmouse"),
                                      i18n("Pointer device KCM"),
                                      QStringLiteral("1.0"),
                                      i18n("System Settings module for managing mice and trackballs."),
                                      KAboutLicense::GPL_V2,
                                      i18n("Copyright 2018 Roman Gilg"),
                                      QString());

    data->addAuthor(i18n("Roman Gilg"), i18n("Developer"), QStringLiteral("subdiff@gmail.com"));

    m_parent->setAboutData(data);

    m_initError = !m_backend->errorString().isNull();

    m_view = new QQuickWidget(this);

    m_errorMessage = new KMessageWidget(this);
    m_errorMessage->setCloseButtonVisible(false);
    m_errorMessage->setWordWrap(true);
    m_errorMessage->setVisible(false);

    QVBoxLayout *layout = new QVBoxLayout(parent);

    layout->addWidget(m_errorMessage);
    layout->addWidget(m_view);
    parent->setLayout(layout);

    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_view->setClearColor(Qt::transparent);
    m_view->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_view->rootContext()->setContextProperty("backend", m_backend);
    m_view->rootContext()->setContextProperty("deviceModel", getDeviceList(m_backend));

    KDeclarative::KDeclarative kdeclarative;
    kdeclarative.setupEngine(m_view->engine()); // This is a new engine
    kdeclarative.setDeclarativeEngine(m_view->engine());
    kdeclarative.setupContext();

    if (m_backend->mode() == InputBackendMode::XLibinput) {
        m_view->setSource(QUrl("qrc:/libinput/main_deviceless.qml"));
    } else {
        m_view->setSource(QUrl("qrc:/libinput/main.qml"));
    }

    if (m_initError) {
        m_errorMessage->setMessageType(KMessageWidget::Error);
        m_errorMessage->setText(m_backend->errorString());
        QMetaObject::invokeMethod(m_errorMessage, "animatedShow", Qt::QueuedConnection);
    } else {
        connect(m_backend, SIGNAL(deviceAdded(bool)), this, SLOT(onDeviceAdded(bool)));
        connect(m_backend, SIGNAL(deviceRemoved(int)), this, SLOT(onDeviceRemoved(int)));
        connect(m_view->rootObject(), SIGNAL(changeSignal()), this, SLOT(onChange()));
    }

    m_view->show();
}

QSize LibinputConfig::sizeHint() const
{
    return QQmlProperty::read(m_view->rootObject(), "sizeHint").toSize();
}

QSize LibinputConfig::minimumSizeHint() const
{
    return QQmlProperty::read(m_view->rootObject(), "minimumSizeHint").toSize();
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
    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
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
    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
}

void LibinputConfig::onChange()
{
    if (!m_backend->deviceCount()) {
        return;
    }
    hideErrorMessage();
    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
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

    Q_EMIT m_parent->changed(m_backend->isChangedConfig());
}

void LibinputConfig::hideErrorMessage()
{
    if (m_errorMessage->isVisible()) {
        m_errorMessage->animatedHide();
    }
}
