/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TOUCHPADCONFIGXLIB_H
#define TOUCHPADCONFIGXLIB_H

#include "../touchpadconfigplugin.h"

#include <KConfigDialogManager>
#include <QScopedPointer>
#include <QSpinBox>

#include "kdedsettings.h"
#include "testarea.h"
#include "touchpadparameters.h"

#include "ui_kded.h"
#include "ui_pointermotion.h"
#include "ui_scroll.h"
#include "ui_sensitivity.h"
#include "ui_tap.h"

class TouchpadConfigContainer;
class TouchpadBackend;
class KMessageWidget;
class OrgKdeTouchpadInterface;
class CustomConfigDialogManager;
class QAction;
class KShortcutsDialog;
class QTabWidget;
class KComboBox;
class QDBusPendingCallWatcher;
class QHideEvent;

class TouchpadConfigXlib : public TouchpadConfigPlugin
{
    Q_OBJECT

public:
    explicit TouchpadConfigXlib(TouchpadConfigContainer *parent, TouchpadBackend *backend, const QVariantList &args = QVariantList());
    ~TouchpadConfigXlib() override;

    static void kcmInit();

    void load() override;
    void save() override;
    void defaults() override;

    void hideEvent(QHideEvent *) override;

private Q_SLOTS:
    void beginTesting();
    void endTesting();
    void onChanged();
    void checkChanges();
    void loadActiveConfig();
    void updateTestAreaEnabled();
    void updateMouseList();
    void showConfigureNotificationsDialog();
    void gotReplyFromDaemon(QDBusPendingCallWatcher *);

private:
    QVariantHash getActiveConfig();

    TouchpadParameters m_config;

    QScopedPointer<QVariantHash> m_prevConfig;
    CustomConfigDialogManager *m_manager;
    TouchpadDisablerSettings m_daemonSettings;
    KConfigDialogManager *m_daemonConfigManager;
    KMessageWidget *m_errorMessage, *m_configOutOfSyncMessage;
    TestArea *m_testArea;
    OrgKdeTouchpadInterface *m_daemon;
    QAction *m_loadActiveConfiguration;
    bool m_configOutOfSync;
    QScopedPointer<KShortcutsDialog> m_shortcutsDialog;
    QWidget *m_kdedTab;
    QTabWidget *m_tabs;
    KComboBox *m_mouseCombo;

    Ui::PointerMotionForm m_pointerMotion;
    Ui::TapForm m_tapping;
    Ui::ScrollForm m_scrolling;
    Ui::SensitivityForm m_sensitivity;
    Ui::KdedForm m_kded;
};

#endif // TOUCHPADCONFIGXLIB_H
