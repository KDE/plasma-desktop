/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCM_KEYBOARD_WIDGET_H_
#define KCM_KEYBOARD_WIDGET_H_

#include "ui_kcm_keyboard.h"

#include <QTabWidget>

#include <config-keyboard.h>
#include "keyboard_config.h"

class QWidget;
class KeyboardConfig;
class WorkspaceOptions;
struct Rules;
class Flags;
class QString;
class QPushButton;
class LayoutsTableModel;
class KCMiscKeyboardWidget;
class KeyboardLayoutActionCollection;
class KeyboardMiscSettings;

class KCMKeyboardWidget : public QTabWidget
{
    Q_OBJECT

public:
    KCMKeyboardWidget(Rules *rules, KeyboardConfig *keyboardConfig, WorkspaceOptions &workspaceOptions, KCMiscKeyboardWidget *kcmMiscWidget, const QVariantList &args, QWidget *parent = nullptr);
    ~KCMKeyboardWidget() override;

    void updateUI(); // load
    void save();
    void defaults();

    bool isSaveNeeded() const;
    bool isDefault() const;

Q_SIGNALS:
    void changed(bool state);

public Q_SLOTS:
    void setDefaultIndicator(bool visible);

private Q_SLOTS:
    void updateUiDefaultIndicator();
    void addLayout();
    void removeLayout();
    void layoutSelectionChanged();
    // Set move UI values to config
    void uiChanged();
    void scrollToGroupShortcut();
    void scrollTo3rdLevelShortcut();
    void clearGroupShortcuts();
    void clear3rdLevelShortcuts();
    void updateXkbShortcutsButtons();
    void moveUp();
    void moveDown();
    void configureLayoutsChanged();
    void configureXkbOptionsChanged();
    void previewLayout();
    void alternativeShortcutChanged(const QKeySequence &seq);
    void switchKeyboardShortcutChanged();

private:
    Rules *rules;
    Flags *flags;
    Ui::TabWidget *uiWidget;
    KeyboardConfig *keyboardConfig;
    WorkspaceOptions &m_workspaceOptions;
    KeyboardLayoutActionCollection *actionCollection;
    LayoutsTableModel *layoutsTableModel;
    bool uiUpdating;
    bool m_highlightVisible = false;

    void initializeLayoutsUI();
    void initializeXkbOptionsUI();
    void initializeKeyboardModelUI();
    void updateHardwareUI(const QString &model);
    void updateLayoutsUI();
    void updateShortcutsUI();
    void updateSwitchingPolicyUI(KeyboardConfig::SwitchingPolicy policy);
    void updateXkbShortcutButton(const QString &groupName, QPushButton *button);
    void clearXkbGroup(const QString &groupName);
    void moveSelectedLayouts(int shift);
    void populateWithCurrentLayouts();
    void populateWithCurrentXkbOptions();
    void updateLoopCount();
    void handleParameters(const QVariantList &args);
    void saveXkbOptions();
    QStringList xkbOptionsFromUI() const;

    QString keyboardModelFromUI() const;
    KeyboardConfig::SwitchingPolicy switchingPolicyFromUI() const;
    void setDefaultIndicatorVisible(QWidget *widget, bool visible);
};

#endif /* KCM_KEYBOARD_WIDGET_H_ */
