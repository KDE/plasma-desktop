/*
    SPDX-FileCopyrightText: 2017 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcm.h"

#include "logging.h"
#include "touchpadbackend.h"
#include "touchpadmoduledata.h"
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

K_PLUGIN_FACTORY_WITH_JSON(KCMTouchpadFactory, "kcm_touchpad.json", registerPlugin<KCMTouchpad>(); registerPlugin<TouchpadModuleData>();)

extern "C" {
Q_DECL_EXPORT void kcminit()
{
#if BUILD_KCM_TOUCHPAD_X11
    if (KWindowSystem::isPlatformX11()) {
        KCMTouchpad::kcmInit();
    }
#endif
}
}

KCMTouchpad::KCMTouchpad(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    const auto uri = "org.kde.plasma.private.kcm_touchpad";
    qmlRegisterUncreatableType<LibinputCommon>(uri, 1, 0, "InputDevice", QString());

    m_backend = TouchpadBackend::implementation();

    m_initError = !m_backend->errorString().isNull();

    QQmlEngine *engine = qApp->property("__qmlEngine").value<QQmlEngine *>();
    if (engine) {
        m_view = new QQuickWidget(engine, widget());
    } else {
        m_view = new QQuickWidget(widget());
    }

    QVBoxLayout *layout = new QVBoxLayout(widget());

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_view);
    widget()->setLayout(layout);

    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_view->setClearColor(Qt::transparent);
    m_view->setAttribute(Qt::WA_AlwaysStackOnTop);

    m_view->rootContext()->setContextProperty("backend", m_backend);

    QObject::connect(m_view, &QQuickWidget::statusChanged, [&](QQuickWidget::Status status) {
        if (status == QQuickWidget::Ready) {
            connect(m_view->rootObject(), SIGNAL(changeSignal()), this, SLOT(onChange()));
        }
    });

    qmlRegisterSingletonInstance("org.kde.touchpad.kcm", 1, 0, "KCMTouchpad", this);

    m_view->engine()->rootContext()->setContextObject(new KLocalizedContext(m_view->engine()));
    m_view->setSource(QUrl("qrc:/kcm/kcm_touchpad/main.qml"));
    m_view->resize(QSize(500, 600));

    if (m_initError) {
        Q_EMIT showMessage(m_backend->errorString());
    } else {
        connect(m_backend, &TouchpadBackend::deviceAdded, this, &KCMTouchpad::onDeviceAdded);
        connect(m_backend, &TouchpadBackend::deviceRemoved, this, &KCMTouchpad::onDeviceRemoved);
    }

    setButtons(KCModule::Default | KCModule::Apply);
}

void KCMTouchpad::kcmInit()
{
#if BUILD_KCM_TOUCHPAD_X11
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (backend->getMode() == TouchpadInputBackendMode::XLibinput) {
        backend->load();
        backend->save();
    }
#endif
}

void KCMTouchpad::load()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->load()) {
        Q_EMIT showMessage(i18n("Error while loading values. See logs for more information. Please restart this configuration module."));
    } else {
        if (!m_backend->deviceCount()) {
            Q_EMIT showMessage(i18n("No touchpad found. Connect touchpad now."));
        }
    }
    setNeedsSave(false);
}

void KCMTouchpad::save()
{
    if (!m_backend->save()) {
        Q_EMIT showMessage(i18n("Not able to save all changes. See logs for more information. Please restart this configuration module and try again."));
    } else {
        Q_EMIT showMessage(QString());
    }

    // load newly written values
    load();
    // in case of error, config still in changed state
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::defaults()
{
    // in case of critical init error in backend, don't try
    if (m_initError) {
        return;
    }

    if (!m_backend->defaults()) {
        Q_EMIT showMessage(i18n("Error while loading default values. Failed to set some options to their default values."));
    }
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::onChange()
{
    if (m_backend->deviceCount() > 0) {
        hideErrorMessage();
    }
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::onDeviceAdded(bool success)
{
    if (!success) {
        Q_EMIT showMessage(i18n("Error while adding newly connected device. Please reconnect it and restart this configuration module."));
    }

    if (m_backend->deviceCount() > 0) {
        hideErrorMessage();
    }
}

void KCMTouchpad::onDeviceRemoved(int index)
{
    QQuickItem *rootObj = m_view->rootObject();

    int activeIndex = QQmlProperty::read(rootObj, "deviceIndex").toInt();
    if (activeIndex == index) {
        if (m_backend->deviceCount() > 0) {
            Q_EMIT showMessage(i18n("Touchpad disconnected. Closed its setting dialog."), 0 /*Kirigami.MessageType.Information*/);
        } else {
            Q_EMIT showMessage(i18n("Touchpad disconnected. No other touchpads found."), 0 /*Kirigami.MessageType.Information*/);
        }
    }
    setNeedsSave(m_backend->isSaveNeeded());
}

void KCMTouchpad::hideErrorMessage()
{
    Q_EMIT showMessage(QString());
}

#include "kcm.moc"
#include "moc_kcm.cpp"
