/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCM_KEYBOARD_H_
#define KCM_KEYBOARD_H_

#include "workspace_options.h"
#include <KCModule>

class KCMKeyboardWidget;
class KCMiscKeyboardWidget;
class KeyboardSettingsData;
struct Rules;

class KCMKeyboard : public KCModule
{
    Q_OBJECT

public:
    KCMKeyboard(QWidget *parent, const QVariantList &);
    ~KCMKeyboard() override;

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;
    void updateUnmanagedState();

private:
    Rules *rules;
    KeyboardSettingsData *m_data;
    WorkspaceOptions m_workspaceOptions;
    KCMKeyboardWidget *widget;
    KCMiscKeyboardWidget *m_miscWidget;
};

#endif /* KCM_KEYBOARD_H_ */
