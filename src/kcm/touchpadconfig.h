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

#ifndef TOUCHPADCONFIG_H
#define TOUCHPADCONFIG_H

#include <KCModule>
#include <KConfigDialogManager>
#include <QScopedPointer>

#include "touchpadparameters.h"
#include "testarea.h"
#include "kdedsettings.h"

#include "ui_pointermotion.h"
#include "ui_tap.h"
#include "ui_scroll.h"
#include "ui_sensitivity.h"
#include "ui_kded.h"

class TouchpadBackend;
class KMessageWidget;
class OrgKdeTouchpadInterface;
class CustomConfigDialogManager;
class KAction;
class KShortcutsDialog;
class KTabWidget;
class KComboBox;
class QDBusPendingCallWatcher;

class TouchpadConfig : public KCModule
{
    Q_OBJECT

public:
    explicit TouchpadConfig(QWidget *parent,
                            const QVariantList &args = QVariantList());
    virtual ~TouchpadConfig();

    virtual void load();
    virtual void save();
    virtual void defaults();

protected:
    virtual void hideEvent(QHideEvent *);

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

    TouchpadBackend *m_backend;
    TouchpadParameters m_config;

    QScopedPointer<QVariantHash> m_prevConfig;
    CustomConfigDialogManager *m_manager;
    TouchpadDisablerSettings m_daemonSettings;
    KConfigDialogManager *m_daemonConfigManager;
    KMessageWidget *m_errorMessage, *m_configOutOfSyncMessage;
    TestArea *m_testArea;
    OrgKdeTouchpadInterface *m_daemon;
    KAction *m_loadActiveConfiguration;
    bool m_configOutOfSync;
    QScopedPointer<KShortcutsDialog> m_shortcutsDialog;
    QWidget *m_kdedTab;
    KTabWidget *m_tabs;
    KComboBox *m_mouseCombo;

    Ui::PointerMotionForm m_pointerMotion;
    Ui::TapForm m_tapping;
    Ui::ScrollForm m_scrolling;
    Ui::SensitivityForm m_sensitivity;
    Ui::KdedForm m_kded;
};

#endif // TOUCHPADCONFIG_H
