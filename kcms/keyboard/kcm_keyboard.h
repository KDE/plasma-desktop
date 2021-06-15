/*
    SPDX-FileCopyrightText: 2010 Andriy Rysin <rysin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KCM_KEYBOARD_H_
#define KCM_KEYBOARD_H_

#include "workspace_options.h"
#include <KCModule>

class KCMKeyboardWidget;
class KeyboardConfig;
struct Rules;

class KCMKeyboard : public KCModule
{
    Q_OBJECT

public:
    KCMKeyboard(QWidget *parent, const QVariantList &);
    ~KCMKeyboard() override;

    void save() override;
    void load() override;
    void defaults() override;

private:
    Rules *rules;
    KeyboardConfig *keyboardConfig;
    WorkspaceOptions m_workspaceOptions;
    KCMKeyboardWidget *widget;
};

#endif /* KCM_KEYBOARD_H_ */
