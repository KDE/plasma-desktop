/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "touchpadconfigxlib.h"

#include <QDBusInterface>
#include <QScrollArea>
#include <QTabWidget>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageWidget>
#include <KNotifyConfigWidget>
#include <KShortcutsDialog>
#include <QAction>

#include "../touchpadconfigcontainer.h"
#include "customconfigdialogmanager.h"
#include "customslider.h"
#include "actions.h"
#include "sliderpair.h"
#include "touchpadbackend.h"
#include "touchpadinterface.h"

#include "plasma_version.h"

void TouchpadConfigXlib::kcmInit()
{
    TouchpadParameters::setSystemDefaults();
    touchpadApplySavedConfig();
}

static void copyHelpFromBuddy(QObject *root)
{
    QLabel *asLabel = qobject_cast<QLabel *>(root);
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
    const auto childList = root->children();
    for (QObject *child : childList) {
        copyHelpFromBuddy(child);
    }
}

template<typename T>
QWidget *addTab(QTabWidget *tabs, T &form)
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

TouchpadConfigXlib::TouchpadConfigXlib(TouchpadConfigContainer *parent, TouchpadBackend *backend, const QVariantList & /*args*/)
    : TouchpadConfigPlugin(parent, backend)
    , m_configOutOfSync(false)
{
    KAboutData *data = new KAboutData(QStringLiteral("kcm_touchpad"),
                                      i18n("Touchpad KCM"),
                                      PLASMA_VERSION_STRING,
                                      i18n("System Settings module, daemon and Plasma applet for managing your touchpad"),
                                      KAboutLicense::GPL_V2,
                                      i18n("Copyright © 2013 Alexander Mezin"),
                                      i18n("This program incorporates work covered by this copyright notice:\n"
                                           "Copyright © 2002-2005,2007 Peter Osterlund"),
                                      QStringLiteral("https://projects.kde.org/projects/playground/utils/kcm-touchpad/"),
                                      QString());

    data->addAuthor(i18n("Alexander Mezin"), i18n("Developer"), QStringLiteral("mezin.alexander@gmail.com"));
    data->addCredit(i18n("Thomas Pfeiffer"), i18nc("Credits", "Usability, testing"));
    data->addCredit(i18n("Alex Fiestas"), i18nc("Credits", "Helped a bit"));
    data->addCredit(i18n("Peter Osterlund"), i18nc("Credits", "Developer of synclient"));
    data->addCredit(i18n("Vadim Zaytsev"), i18nc("Credits", "Testing"));
    data->addCredit(i18n("Violetta Raspryagayeva"), i18nc("Credits", "Testing"));

    m_parent->setAboutData(data);

    QGridLayout *layout = new QGridLayout(this);
    QVBoxLayout *messageLayout = new QVBoxLayout();
    layout->addLayout(messageLayout, 0, 0, 1, 2);

    // Messages

    m_errorMessage = new KMessageWidget(this);
    m_errorMessage->setMessageType(KMessageWidget::Error);
    m_errorMessage->setVisible(false);
    m_errorMessage->setWordWrap(true);
    messageLayout->addWidget(m_errorMessage);

    m_configOutOfSyncMessage = new KMessageWidget(this);
    m_configOutOfSyncMessage->setMessageType(KMessageWidget::Warning);
    m_configOutOfSyncMessage->setText(
        i18n("Active settings don't match saved settings.\n"
             "You currently see saved settings."));
    m_configOutOfSyncMessage->setVisible(false);
    messageLayout->addWidget(m_configOutOfSyncMessage);

    m_loadActiveConfiguration = new QAction(m_configOutOfSyncMessage);
    m_loadActiveConfiguration->setText(i18n("Show active settings"));
    connect(m_loadActiveConfiguration, SIGNAL(triggered()), SLOT(loadActiveConfig()));
    m_configOutOfSyncMessage->addAction(m_loadActiveConfiguration);

    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 1);

    // Main UI

    m_tabs = new QTabWidget(this);
    layout->addWidget(m_tabs, 1, 0, 1, 1);

    addTab(m_tabs, m_tapping);
    addTab(m_tabs, m_scrolling);
    addTab(m_tabs, m_pointerMotion);
    addTab(m_tabs, m_sensitivity);

    static const CustomSlider::SqrtInterpolator interpolator;
    m_pointerMotion.kcfg_MinSpeed->setInterpolator(&interpolator);
    m_pointerMotion.kcfg_MaxSpeed->setInterpolator(&interpolator);
    m_pointerMotion.kcfg_AccelFactor->setInterpolator(&interpolator);

    new SliderPair(m_pointerMotion.kcfg_MinSpeed, m_pointerMotion.kcfg_MaxSpeed, this);
    new SliderPair(m_sensitivity.kcfg_FingerLow, m_sensitivity.kcfg_FingerHigh, this);
    new SliderPair(m_pointerMotion.kcfg_PressureMotionMinZ, m_pointerMotion.kcfg_PressureMotionMaxZ, this);

    KConfigDialogManager::changedMap()->insert("CustomSlider", SIGNAL(valueChanged(double)));
    m_manager = new CustomConfigDialogManager(this, &m_config, m_backend->supportedParameters());
    connect(m_manager, SIGNAL(widgetModified()), SLOT(checkChanges()), Qt::QueuedConnection);

    // KDED settings

    m_kdedTab = addTab(m_tabs, m_kded);
    m_daemonConfigManager = m_parent->addConfig(&m_daemonSettings, m_kdedTab);

    KMessageWidget *kdedMessage = new KMessageWidget(m_kdedTab);
    kdedMessage->setMessageType(KMessageWidget::Information);
    kdedMessage->setCloseButtonVisible(false);
    kdedMessage->setText(i18n("These settings won't take effect in the testing area"));
    qobject_cast<QVBoxLayout *>(m_kdedTab->layout())->insertWidget(0, kdedMessage);

    connect(m_kded.configureNotificationsButton, SIGNAL(clicked()), SLOT(showConfigureNotificationsDialog()));
    m_shortcutsDialog.reset(new KShortcutsDialog(KShortcutsEditor::GlobalAction, KShortcutsEditor::LetterShortcutsDisallowed));
    m_shortcutsDialog->addCollection(new TouchpadGlobalActions(true, this), i18n("Enable/Disable Touchpad"));
    connect(m_kded.configureShortcutsButton, &QPushButton::clicked, this, [this]() {
        m_shortcutsDialog->configure(true);
    });

    m_mouseCombo = new KComboBox(true, m_kded.kcfg_MouseBlacklist);
    m_kded.kcfg_MouseBlacklist->setCustomEditor(m_mouseCombo);
    connect(m_backend, SIGNAL(mousesChanged()), SLOT(updateMouseList()));
    m_backend->watchForEvents(false);
    updateMouseList();

    m_daemon = new OrgKdeTouchpadInterface("org.kde.kded5", "/modules/touchpad", QDBusConnection::sessionBus(), this);
    m_kdedTab->setEnabled(false);
    QDBusPendingCallWatcher *watch;
    watch = new QDBusPendingCallWatcher(m_daemon->workingTouchpadFound(), this);
    // clang-format off
    connect(watch, SIGNAL(finished(QDBusPendingCallWatcher*)), SLOT(gotReplyFromDaemon(QDBusPendingCallWatcher*)));
    // clang-format on

    // Testing area

    m_testArea = new TestArea(this);
    layout->addWidget(m_testArea, 1, 1);
    connect(m_testArea, SIGNAL(enter()), SLOT(beginTesting()));
    connect(m_testArea, SIGNAL(leave()), SLOT(endTesting()));
    connect(m_tabs, SIGNAL(currentChanged(int)), SLOT(updateTestAreaEnabled()));
    updateTestAreaEnabled();
}

void TouchpadConfigXlib::gotReplyFromDaemon(QDBusPendingCallWatcher *watch)
{
    QDBusPendingReply<bool> reply = *watch;
    if (reply.isValid() && reply.value()) {
        m_kdedTab->setEnabled(true);
    }
    watch->deleteLater();
}

void TouchpadConfigXlib::updateMouseList()
{
    const QStringList mouses(m_backend->listMouses(m_daemonSettings.mouseBlacklist()));

    for (int i = 0; i < m_mouseCombo->count();) {
        if (!mouses.contains(m_mouseCombo->itemText(i))) {
            m_mouseCombo->removeItem(i);
        } else {
            i++;
        }
    }

    for (const QString &device : mouses) {
        if (!m_mouseCombo->contains(device)) {
            m_mouseCombo->addItem(device);
        }
    }
}

QVariantHash TouchpadConfigXlib::getActiveConfig()
{
    if (m_prevConfig) {
        return *m_prevConfig;
    }

    QVariantHash activeConfig;
    if (!m_backend->getConfig(activeConfig)) {
        m_errorMessage->setText(m_backend->errorString());
        QMetaObject::invokeMethod(m_errorMessage, "animatedShow", Qt::QueuedConnection);
    }
    return activeConfig;
}

void TouchpadConfigXlib::loadActiveConfig()
{
    m_manager->setWidgetProperties(getActiveConfig());
    m_configOutOfSync = false;
    m_configOutOfSyncMessage->animatedHide();
}

void TouchpadConfigXlib::load()
{
    m_manager->updateWidgets();

    m_parent->kcmLoad();

    m_configOutOfSync = !m_manager->compareWidgetProperties(getActiveConfig());
}

void TouchpadConfigXlib::save()
{
    m_manager->updateSettings();

    m_configOutOfSync = false;
    m_configOutOfSyncMessage->animatedHide();

    bool daemonSettingsChanged = m_daemonConfigManager->hasChanged();

    m_parent->kcmSave();

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

void TouchpadConfigXlib::defaults()
{
    m_manager->updateWidgetsDefault();

    m_parent->kcmDefaults();
}

void TouchpadConfigXlib::checkChanges()
{
    if (!m_backend->touchpadCount()) {
        return;
    }
    m_parent->unmanagedWidgetChangeState(m_manager->hasChangedFuzzy() || m_configOutOfSync);
    if (m_configOutOfSync) {
        m_configOutOfSyncMessage->animatedShow();
    } else {
        m_configOutOfSyncMessage->animatedHide();
    }
}

void TouchpadConfigXlib::hideEvent(QHideEvent *e)
{
    Q_UNUSED(e);
    endTesting();
}

TouchpadConfigXlib::~TouchpadConfigXlib()
{
    endTesting();
}

void TouchpadConfigXlib::onChanged()
{
    if (m_testArea->underMouse()) {
        beginTesting();
    }
}

void TouchpadConfigXlib::beginTesting()
{
    if (!m_prevConfig) {
        m_prevConfig.reset(new QVariantHash());
        m_backend->getConfig(*m_prevConfig.data());
    }
    m_backend->applyConfig(m_manager->currentWidgetProperties());
}

void TouchpadConfigXlib::endTesting()
{
    if (!m_prevConfig) {
        return;
    }
    m_backend->applyConfig(*m_prevConfig.data());
    m_prevConfig.reset();
}

void TouchpadConfigXlib::updateTestAreaEnabled()
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

void TouchpadConfigXlib::showConfigureNotificationsDialog()
{
    KNotifyConfigWidget *widget = KNotifyConfigWidget::configure(nullptr, m_parent->componentData().componentName());
    QDialog *dialog = qobject_cast<QDialog *>(widget->topLevelWidget());
    connect(dialog, SIGNAL(finished()), dialog, SLOT(deleteLater()));
}
