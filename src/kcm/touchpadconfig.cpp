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
#include <KAction>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KNotifyConfigWidget>
#include <KShortcutsDialog>
#include <KTabWidget>

#include "customslider.h"
#include "sliderpair.h"
#include "touchpadbackend.h"
#include "plugins.h"
#include "testarea.h"
#include "touchpadinterface.h"
#include "customconfigdialogmanager.h"
#include "kded/kdedactions.h"

void touchpadApplySavedConfig()
{
    TouchpadBackend *backend = TouchpadBackend::implementation();
    if (!backend) {
        return;
    }

    TouchpadParameters config;
    backend->applyConfig(config.values());
}

extern "C"
{
    KDE_EXPORT void kcminit_touchpad()
    {
        TouchpadParameters::setSystemDefaults();
        touchpadApplySavedConfig();
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
QWidget *addTab(KTabWidget *tabs, T &form)
{
    QScrollArea *container = new QScrollArea(tabs);
    container->setWidgetResizable(true);
    container->setFrameStyle(QFrame::NoFrame);
    container->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    QWidget *widget = new QWidget(container);
    form.setupUi(widget);
    copyHelpFromBuddy(widget);
    widget->setContentsMargins(20, 20, 20, 20);
    widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    container->setWidget(widget);
    tabs->addTab(container, widget->windowTitle());

    return widget;
}

TouchpadConfig::TouchpadConfig(QWidget *parent, const QVariantList &args)
    : KCModule(TouchpadPluginFactory::componentData(), parent, args),
      m_configOutOfSync(false)
{
    setAboutData(new KAboutData(*componentData().aboutData()));

    QGridLayout *layout = new QGridLayout(this);
    QVBoxLayout *messageLayout = new QVBoxLayout();
    layout->addLayout(messageLayout, 0, 0, 1, 2);

    // Messages

    m_errorMessage = new KMessageWidget(this);
    m_errorMessage->setMessageType(KMessageWidget::Error);
    m_errorMessage->setVisible(false);
    messageLayout->addWidget(m_errorMessage);

    m_configOutOfSyncMessage = new KMessageWidget(this);
    m_configOutOfSyncMessage->setMessageType(KMessageWidget::Warning);
    m_configOutOfSyncMessage->setText(
                i18n("Active settings don't match saved settings.\n"
                     "You currently see saved settings."));
    m_configOutOfSyncMessage->setVisible(false);
    messageLayout->addWidget(m_configOutOfSyncMessage);

    m_loadActiveConfiguration = new KAction(m_configOutOfSyncMessage);
    m_loadActiveConfiguration->setText(i18n("Show active settings"));
    connect(m_loadActiveConfiguration, SIGNAL(triggered()),
            SLOT(loadActiveConfig()));
    m_configOutOfSyncMessage->addAction(m_loadActiveConfiguration);

    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 1);

    // Main UI

    m_tabs = new KTabWidget(this);
    layout->addWidget(m_tabs, 1, 0, 1, 1);

    addTab(m_tabs, m_tapping);
    addTab(m_tabs, m_scrolling);
    addTab(m_tabs, m_pointerMotion);
    addTab(m_tabs, m_sensitivity);

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

    m_backend = TouchpadBackend::implementation();

    KConfigDialogManager::changedMap()->insert("CustomSlider",
                                               SIGNAL(valueChanged(double)));
    m_manager = new CustomConfigDialogManager(this, &m_config,
                                              m_backend->supportedParameters());
    connect(m_manager, SIGNAL(widgetModified()), SLOT(checkChanges()),
            Qt::QueuedConnection);

    // KDED settings

    m_kdedTab = addTab(m_tabs, m_kded);
    m_daemonConfigManager = addConfig(&m_daemonSettings, m_kdedTab);

    KMessageWidget *kdedMessage = new KMessageWidget(m_kdedTab);
    kdedMessage->setMessageType(KMessageWidget::Information);
    kdedMessage->setCloseButtonVisible(false);
    kdedMessage->setText(
                i18n("These settings won't take effect in the testing area"));
    qobject_cast<QVBoxLayout *>(m_kdedTab->layout())->
            insertWidget(0, kdedMessage);

    connect(m_kded.configureNotificationsButton, SIGNAL(clicked()),
            SLOT(showConfigureNotificationsDialog()));
    m_shortcutsDialog.reset(new KShortcutsDialog(KShortcutsEditor::GlobalAction,
                                                 KShortcutsEditor::LetterShortcutsDisallowed));
    m_shortcutsDialog->addCollection(new TouchpadGlobalActions(this),
                                     i18n("Enable/Disable Touchpad"));
    connect(m_kded.configureShortcutsButton, SIGNAL(clicked()),
            m_shortcutsDialog.data(), SLOT(show()));

    m_mouseCombo = new KComboBox(true, m_kded.kcfg_MouseBlacklist);
    m_kded.kcfg_MouseBlacklist->setCustomEditor(m_mouseCombo);
    connect(m_backend, SIGNAL(mousesChanged()), SLOT(updateMouseList()));
    m_backend->watchForEvents(false);
    updateMouseList();

    m_daemon = new OrgKdeTouchpadInterface("org.kde.kded", "/modules/touchpad",
                                           QDBusConnection::sessionBus(), this);
    m_kdedTab->setEnabled(false);
    QDBusPendingCallWatcher *watch;
    watch = new QDBusPendingCallWatcher(m_daemon->workingTouchpadFound(), this);
    connect(watch, SIGNAL(finished(QDBusPendingCallWatcher*)),
            SLOT(gotReplyFromDaemon(QDBusPendingCallWatcher*)));

    // Testing area

    m_testArea = new TestArea(this);
    layout->addWidget(m_testArea, 1, 1);
    connect(m_testArea, SIGNAL(enter()), SLOT(beginTesting()));
    connect(m_testArea, SIGNAL(leave()), SLOT(endTesting()));
    connect(this, SIGNAL(changed(bool)), SLOT(onChanged()));
    connect(m_tabs, SIGNAL(currentChanged(int)), SLOT(updateTestAreaEnabled()));
    updateTestAreaEnabled();
}

void TouchpadConfig::gotReplyFromDaemon(QDBusPendingCallWatcher *watch)
{
    QDBusPendingReply<bool> reply = *watch;
    if (reply.isValid() && reply.value()) {
        m_kdedTab->setEnabled(true);
    }
    watch->deleteLater();
}

void TouchpadConfig::updateMouseList()
{
    QStringList mouses(
                m_backend->listMouses(m_daemonSettings.mouseBlacklist()));

    for (int i = 0; i < m_mouseCombo->count(); ) {
        if (!mouses.contains(m_mouseCombo->itemText(i))) {
            m_mouseCombo->removeItem(i);
        } else {
            i++;
        }
    }

    Q_FOREACH (const QString &i, mouses) {
        if (!m_mouseCombo->contains(i)) {
            m_mouseCombo->addItem(i);
        }
    }
}

QVariantHash TouchpadConfig::getActiveConfig()
{
    if (m_prevConfig) {
        return *m_prevConfig;
    }

    QVariantHash activeConfig;
    if (!m_backend->getConfig(activeConfig)) {
        m_errorMessage->setText(m_backend->errorString());
        QMetaObject::invokeMethod(m_errorMessage, "animatedShow",
                                  Qt::QueuedConnection);
    }
    return activeConfig;
}

void TouchpadConfig::loadActiveConfig()
{
    m_manager->setWidgetProperties(getActiveConfig());
    m_configOutOfSync = false;
    m_configOutOfSyncMessage->animatedHide();
}

void TouchpadConfig::load()
{
    m_manager->updateWidgets();

    KCModule::load();

    m_configOutOfSync = !m_manager->compareWidgetProperties(getActiveConfig());
}

void TouchpadConfig::save()
{
    m_manager->updateSettings();

    m_configOutOfSync = false;
    m_configOutOfSyncMessage->animatedHide();

    bool daemonSettingsChanged = m_daemonConfigManager->hasChanged();

    KCModule::save();

    if (m_backend->applyConfig(m_config.values())) {
        m_errorMessage->animatedHide();
    } else {
        m_errorMessage->setText(m_backend->errorString());
        m_errorMessage->animatedShow();
    }

    if (daemonSettingsChanged) {
        m_daemon->reloadSettings();
        updateMouseList();
    }
}

void TouchpadConfig::defaults()
{
    m_manager->updateWidgetsDefault();

    KCModule::defaults();
}

void TouchpadConfig::checkChanges()
{
    unmanagedWidgetChangeState(m_manager->hasChangedFuzzy()
                               || m_configOutOfSync);
    if (m_configOutOfSync) {
        m_configOutOfSyncMessage->animatedShow();
    } else {
        m_configOutOfSyncMessage->animatedHide();
    }
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

void TouchpadConfig::updateTestAreaEnabled()
{
    bool enable = true;
    for (QWidget *i = m_kdedTab; i; i = i->parentWidget()) {
        if (i == m_tabs->currentWidget()) {
            enable = false;
            break;
        }
    }

    m_testArea->setEnabled(enable);
    m_testArea->setMouseTracking(enable);
    if (!enable) {
        endTesting();
    }
}

void TouchpadConfig::showConfigureNotificationsDialog()
{
    KNotifyConfigWidget *widget =
            KNotifyConfigWidget::configure(0, componentData().componentName());
    KDialog *dialog = qobject_cast<KDialog*>(widget->topLevelWidget());
    connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
}
