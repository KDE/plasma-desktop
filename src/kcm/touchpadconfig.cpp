/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "touchpadconfig.h"

#include <QScrollArea>
#include <QDBusInterface>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KTabWidget>

#include "customslider.h"
#include "sliderpair.h"
#include "touchpadbackend.h"
#include "plugins.h"
#include "testarea.h"
#include "touchpadinterface.h"
#include "customconfigdialogmanager.h"

extern "C"
{
    KDE_EXPORT void kcminit_touchpad()
    {
        TouchpadBackend *backend = TouchpadBackend::self();

        if (!backend) {
            return;
        }

        QVariantHash current;
        backend->getConfig(current);
        TouchpadParameters::setSystemDefaults(current);

        TouchpadParameters config;
        backend->applyConfig(config.values());
    }
}

static void copyHelpFromBuddy(QObject *root)
{
    QLabel *asLabel = qobject_cast<QLabel*>(root);
    if (asLabel && asLabel->buddy()) {
        if (asLabel->toolTip().isEmpty()) {
            asLabel->setToolTip(asLabel->buddy()->toolTip());
        }
        if (asLabel->statusTip().isEmpty()) {
            asLabel->setStatusTip(asLabel->buddy()->statusTip());
        }
        if (asLabel->whatsThis().isEmpty()) {
            asLabel->setWhatsThis(asLabel->buddy()->whatsThis());
        }
    }
    Q_FOREACH(QObject *child, root->children()) {
        copyHelpFromBuddy(child);
    }
}

template<typename T>
void addTab(KTabWidget *tabs, T &form)
{
    QScrollArea *container = new QScrollArea(tabs);
    container->setWidgetResizable(true);
    container->setFrameStyle(QFrame::NoFrame);
    container->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QWidget *widget = new QWidget(container);
    form.setupUi(widget);
    copyHelpFromBuddy(widget);
    widget->layout()->setContentsMargins(20, 20, 20, 20);

    container->setWidget(widget);
    tabs->addTab(container, widget->windowTitle());
}

TouchpadConfig::TouchpadConfig(QWidget *parent, const QVariantList &args)
    : KCModule(TouchpadPluginFactory::componentData(), parent, args)
{
    setAboutData(new KAboutData(*componentData().aboutData()));

    QGridLayout *layout = new QGridLayout(this);

    m_errorMessage = new KMessageWidget(this);
    m_errorMessage->setMessageType(KMessageWidget::Error);
    m_errorMessage->setVisible(false);
    layout->addWidget(m_errorMessage, 0, 0, 1, 2);

    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 1);

    KTabWidget *tabs = new KTabWidget(this);

    addTab(tabs, m_tapping);
    addTab(tabs, m_scrolling);
    addTab(tabs, m_pointerMotion);
    addTab(tabs, m_sensitivity);

    layout->addWidget(tabs, 1, 0, 1, 1);

    m_testArea = new TestArea(this);
    layout->addWidget(m_testArea, 1, 1);
    connect(m_testArea, SIGNAL(enter()), SLOT(beginTesting()));
    connect(m_testArea, SIGNAL(leave()), SLOT(endTesting()));
    connect(this, SIGNAL(changed(bool)), SLOT(onChanged()));

    static const CustomSlider::SqrtInterpolator interpolator;
    m_pointerMotion.kcfg_MinSpeed->setInterpolator(&interpolator);
    m_pointerMotion.kcfg_MaxSpeed->setInterpolator(&interpolator);
    m_pointerMotion.kcfg_AccelFactor->setInterpolator(&interpolator);

    new SliderPair(m_pointerMotion.kcfg_MinSpeed,
                   m_pointerMotion.kcfg_MaxSpeed, this);
    new SliderPair(m_sensitivity.kcfg_FingerLow,
                   m_sensitivity.kcfg_FingerHigh, this);
    new SliderPair(m_pointerMotion.kcfg_PressureMotionMinZ,
                   m_pointerMotion.kcfg_PressureMotionMaxZ, this);

    m_backend = TouchpadBackend::self();

    KConfigDialogManager::changedMap()->insert("CustomSlider",
                                               SIGNAL(valueChanged(double)));
    m_manager = new CustomConfigDialogManager(this, &m_config,
                                              m_backend->supportedParameters());
    connect(m_manager, SIGNAL(widgetModified()), SLOT(checkChanges()));
    addConfig(&m_daemonSettings, this);

    KComboBox *mouseCombo = new KComboBox(true, this);
    mouseCombo->addItems(
                m_backend->listMouses(m_daemonSettings.mouseBlacklist()));
    m_sensitivity.kcfg_MouseBlacklist->setCustomEditor(mouseCombo);

    m_daemon = new OrgKdeTouchpadInterface("org.kde.kded", "/modules/touchpad",
                                           QDBusConnection::sessionBus(), this);
}

void TouchpadConfig::load()
{
    m_manager->updateWidgets();

    KCModule::load();
}

void TouchpadConfig::save()
{
    m_manager->updateSettings();

    KCModule::save();

    if (m_backend->applyConfig(m_config.values())) {
        m_errorMessage->animatedHide();
    } else {
        m_errorMessage->setText(m_backend->errorString());
        m_errorMessage->animatedShow();
    }

    m_daemon->reloadSettings();
}

void TouchpadConfig::defaults()
{
    m_manager->updateWidgetsDefault();

    KCModule::defaults();
}

void TouchpadConfig::checkChanges()
{
    unmanagedWidgetChangeState(m_manager->hasChanged());
}

void TouchpadConfig::hideEvent(QHideEvent *e)
{
    endTesting();
    KCModule::hideEvent(e);
}

TouchpadConfig::~TouchpadConfig()
{
    endTesting();
}

void TouchpadConfig::onChanged()
{
    if (m_testArea->underMouse()) {
        beginTesting();
    }
}

void TouchpadConfig::beginTesting()
{
    if (!m_prevConfig) {
        m_prevConfig.reset(new QVariantHash());
        m_backend->getConfig(*m_prevConfig.data());
    }
    m_backend->applyConfig(m_manager->currentWidgetProperties());
}

void TouchpadConfig::endTesting()
{
    if (!m_prevConfig) {
        return;
    }
    m_backend->applyConfig(*m_prevConfig.data());
    m_prevConfig.reset();
}
