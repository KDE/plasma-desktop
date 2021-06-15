/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCM_KEYBOARD_WIDGET_H_
#define KCM_KEYBOARD_WIDGET_H_

#include "ui_kcm_keyboard.h"

#include <QTabWidget>

#include <config-keyboard.h>

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

class KCMKeyboardWidget : public QTabWidget
{
    Q_OBJECT

public:
    KCMKeyboardWidget(Rules *rules, KeyboardConfig *keyboardConfig, WorkspaceOptions &workspaceOptions, const QVariantList &args, QWidget *parent = nullptr);
    ~KCMKeyboardWidget() override;

    void updateUI();
    void save();

    // temp hack
    KCMiscKeyboardWidget *getKcmMiscWidget() const
    {
        return kcmMiscWidget;
    }

Q_SIGNALS:
    void changed(bool state);

private Q_SLOTS:
    void addLayout();
    void removeLayout();
    void layoutSelectionChanged();
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

private:
    Rules *rules;
    Flags *flags;
    Ui::TabWidget *uiWidget;
    KeyboardConfig *keyboardConfig;
    WorkspaceOptions &m_workspaceOptions;
    KeyboardLayoutActionCollection *actionCollection;
    LayoutsTableModel *layoutsTableModel;
    KCMiscKeyboardWidget *kcmMiscWidget;
    bool uiUpdating;

    void initializeLayoutsUI();
    void initializeXkbOptionsUI();
    void initializeKeyboardModelUI();
    void updateHardwareUI();
    void updateLayoutsUI();
    void updateShortcutsUI();
    void updateXkbOptionsUI();
    void updateSwitcingPolicyUI();
    void updateXkbShortcutButton(const QString &groupName, QPushButton *button);
    void clearXkbGroup(const QString &groupName);
    void moveSelectedLayouts(int shift);
    void populateWithCurrentLayouts();
    void populateWithCurrentXkbOptions();
    void updateLoopCount();
    void handleParameters(const QVariantList &args);
};

#endif /* KCM_KEYBOARD_WIDGET_H_ */
